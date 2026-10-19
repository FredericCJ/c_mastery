#ifndef CMASTERY_CONTRACT_BOUNDS_H
#define CMASTERY_CONTRACT_BOUNDS_H
#include "api_depth.h"
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>
typedef enum { AC_OK, AC_INVALID, AC_FULL, AC_OVERLAP, AC_MISSING,
               AC_BACKEND, AC_INTERNAL, AC_NOT_IMPLEMENTED } AcStatus;
typedef struct { unsigned char *data; size_t length, capacity; } AcView;
typedef enum { AC_POLL, AC_FINITE, AC_INFINITE } AcWaitTag;
typedef struct { AcWaitTag tag; unsigned milliseconds; } AcWait;
typedef struct { unsigned start, duration; AcWaitTag tag; } AcDeadline;
typedef struct { const ApiBuffer *owner; size_t offset, count; } AcSlice;
typedef struct { int value; const void *nullable; } AcValue;
typedef struct { AcStatus status; AcValue value; } AcResult;
typedef struct { AcStatus status; size_t consumed, produced; } AcProgress;
#ifndef AC_TRACE_ENABLED
#define AC_TRACE_ENABLED 1
#endif
#if AC_TRACE_ENABLED
#define AC_INSTRUMENT(expression) ((void)(expression))
#else
#define AC_INSTRUMENT(expression) ((void)0)
#endif
AcStatus ac_admit(AcView);
AcStatus ac_timeout(int milliseconds, AcWait *);
AcStatus ac_deadline(unsigned now, AcWait, AcDeadline *);
AcStatus ac_expired(AcDeadline, unsigned now, int *);
AcStatus ac_octets16(const uint_least16_t *, size_t units,
                     unsigned char *, size_t octet_capacity);
AcStatus ac_append_slice(ApiBuffer *, AcSlice);
FStatus ac_append_allow(FBuffer *, size_t source_offset, size_t count);
AcStatus ac_lookup(unsigned key, AcValue *);
AcResult ac_lookup_tagged(unsigned key);
AcProgress ac_ascii_prefix(const unsigned char *, size_t, unsigned char *, size_t);
AcStatus ac_copy_ascii(const unsigned char *, size_t, unsigned char *, size_t,
                       size_t *written, int initialize_failure);
AcStatus ac_checked_add(unsigned amount, unsigned capacity, unsigned *value,
                         unsigned *evaluations);
AcStatus ac_admit_wire(const unsigned char *, size_t count, unsigned *value);
#endif
