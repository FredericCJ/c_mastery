#ifndef QC_CLOSE_OWNER_H
#define QC_CLOSE_OWNER_H
/* C17 representation. Read CONTRACTS.md before constructing or copying it. */
enum QCState { QC_OWNED, QC_RETRY, QC_CLOSED, QC_QUARANTINED, QC_MOVED };
enum QCStatus { QC_OK, QC_AGAIN, QC_UNKNOWN, QC_INVALID, QC_TERMINAL };
typedef struct { int fd; enum QCState state; } QCOwner;
typedef struct { int result, error; } QCRaw;
typedef QCRaw (*QCClose)(void *, int);
typedef struct { enum QCState state; int error; } QCObservation;
/* init requires fresh destination; a successful call transfers exclusive fd ownership. */
enum QCStatus qc_init(QCOwner *, int fd);
/* One call, one backend attempt; cancellation disabled by caller across ownership. */
enum QCStatus qc_once(QCOwner *, QCClose, void *, QCObservation *);
/* Handoff is allowed only while ownership is certain. No syscall is performed. */
enum QCStatus qc_handoff(QCOwner *, int *);
#endif
