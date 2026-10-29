#ifndef QC_CLOSE_OWNER_H
#define QC_CLOSE_OWNER_H
/* C17 representation. Read CONTRACTS.md before constructing or copying it. */
enum qc_state { QC_OWNED, QC_RETRY, QC_CLOSED, QC_QUARANTINED, QC_MOVED };
enum qc_status { QC_OK, QC_AGAIN, QC_UNKNOWN, QC_INVALID, QC_TERMINAL };
struct qc_owner { int fd; enum qc_state state; };
struct qc_raw { int result, error; };
typedef struct qc_raw (*qc_close_fn)(void *, int);
struct qc_observation { enum qc_state state; int error; };
/* init requires fresh destination; a successful call transfers exclusive fd ownership. */
enum qc_status qc_init(struct qc_owner *, int fd);
/* One call, one backend attempt; cancellation disabled by caller across ownership. */
enum qc_status qc_once(struct qc_owner *, qc_close_fn, void *, struct qc_observation *);
/* Handoff is allowed only while ownership is certain. No syscall is performed. */
enum qc_status qc_handoff(struct qc_owner *, int *);
#endif
