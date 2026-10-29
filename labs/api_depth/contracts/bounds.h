#ifndef CMASTERY_CONTRACT_BOUNDS_H
#define CMASTERY_CONTRACT_BOUNDS_H
#include "api_depth.h"
#include "foundation.h"
#include <stddef.h>
#include <stdint.h>
enum ac_status { AC_OK, AC_INVALID, AC_FULL, AC_OVERLAP, AC_MISSING,
               AC_BACKEND, AC_INTERNAL, AC_NOT_IMPLEMENTED };
struct ac_view { unsigned char *data; size_t length, capacity; };
enum ac_wait_tag { AC_POLL, AC_FINITE, AC_INFINITE };
struct ac_wait { enum ac_wait_tag tag; unsigned milliseconds; };
struct ac_deadline { unsigned start, duration; enum ac_wait_tag tag; };
struct ac_slice { const struct api_buffer *owner; size_t offset, count; };
struct ac_value { int value; const void *nullable; };
struct ac_result { enum ac_status status; struct ac_value value; };
struct ac_progress { enum ac_status status; size_t consumed, produced; };
#ifndef AC_TRACE_ENABLED
#define AC_TRACE_ENABLED 1
#endif
#if AC_TRACE_ENABLED
#define AC_INSTRUMENT(expression) ((void)(expression))
#else
#define AC_INSTRUMENT(expression) ((void)0)
#endif
enum ac_status ac_admit(struct ac_view);
enum ac_status ac_timeout(int milliseconds, struct ac_wait *);
enum ac_status ac_deadline(unsigned now, struct ac_wait, struct ac_deadline *);
enum ac_status ac_expired(struct ac_deadline, unsigned now, int *);
enum ac_status ac_octets16(const uint_least16_t *, size_t units,
                     unsigned char *, size_t octet_capacity);
enum ac_status ac_append_slice(struct api_buffer *, struct ac_slice);
enum f_status ac_append_allow(struct f_buffer *, size_t source_offset, size_t count);
enum ac_status ac_lookup(unsigned key, struct ac_value *);
struct ac_result ac_lookup_tagged(unsigned key);
struct ac_progress ac_ascii_prefix(const unsigned char *, size_t, unsigned char *, size_t);
enum ac_status ac_copy_ascii(const unsigned char *, size_t, unsigned char *, size_t,
                       size_t *written, int initialize_failure);
enum ac_status ac_checked_add(unsigned amount, unsigned capacity, unsigned *value,
                         unsigned *evaluations);
enum ac_status ac_admit_wire(const unsigned char *, size_t count, unsigned *value);
#endif
