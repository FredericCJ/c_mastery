#ifndef CMASTERY_LC_H
#define CMASTERY_LC_H
/** @file
 * A bounded byte-library ABI. See CONTRACTS.md for the exercise and
 * ARCHITECTURE.md for the design alternatives. This header is the normative
 * operation contract.
 * @see @ref architecture
 * @see @ref exercise_contract
 */
#include <stddef.h>
#include <stdint.h>
#if defined(__GNUC__)
#define LC_API __attribute__((visibility("default")))
#else
#define LC_API
#endif
#ifdef __cplusplus
extern "C" {
#endif
typedef struct LCBuffer LCBuffer;
typedef enum {
  LC_OK,
  LC_INVALID,
  LC_FULL,
  LC_VERSION,
  LC_ALLOC,
  LC_PENDING,
  LC_CANCELLED,
  LC_BUSY,
  LC_UNIMPLEMENTED
} LCStatus;
typedef struct {
  size_t value;
} LCBytes;
typedef struct {
  size_t value;
} LCElements;
/** Release number (1 or 2), distinct from ABI version 1. */
LC_API unsigned lc_release(void);
/** Write a 16-byte ABI descriptor: bytes 'LC', ABI=1, release, followed by
 * sizeof(void*), sizeof(size_t), _Alignof(size_t), zero; remaining bytes zero.
 * All inputs are bytes, not a cast foreign structure. Requested ABI must be 1.
 * Release1 requires exact capacity16; release2 accepts >=16, leaving tail
 * intact. Any rejection preserves every output byte. Null output is invalid.
 */
LC_API LCStatus lc_info(unsigned requested_abi, unsigned char *out,
                        size_t bytes);
/** Allocate an empty opaque buffer; maximum requested capacity is 8 in
 * release1, 16 in release2. Zero capacity is supported. Return null on
 * invalid/allocation failure. The creating library alone must release it. No
 * private layout crosses the ABI; no function is concurrently callable on the
 * same object.
 */
LC_API LCBuffer *lc_create(size_t capacity_bytes);
/** Append borrowed bytes without text interpretation (embedded zero is data).
 * @param buffer Live library-owned object; may not alias input.
 * @param borrowed_bytes Readable byte span; null is permitted only at length0.
 * @param length_bytes Byte count, never element count.
 * @pre The caller keeps all inputs live for this call; no concurrent mutation.
 * @post Success copies exactly the input and increases length by length_bytes.
 * @return LC_FULL on insufficient capacity, LC_INVALID on null/overlap; every
 * rejection preserves all previous bytes and length. There is no allocation.
 */
LC_API LCStatus lc_append(LCBuffer *buffer, const unsigned char *borrowed_bytes,
                          size_t length_bytes);
/** Borrow internal bytes until the next successful append or paired release.
 * The returned pointer must not be freed. Null buffer returns null; null length
 * output returns null. Empty buffers still return a valid borrowed pointer.
 */
LC_API const unsigned char *lc_view(const LCBuffer *, size_t *length_bytes);
/** Copy to caller-owned, disjoint output, preserving it and *written on
 * failure. Null output is permitted only when no bytes need copying. Written is
 * required.
 */
LC_API LCStatus lc_copy(const LCBuffer *, unsigned char *,
                        size_t capacity_bytes, size_t *written);
/** Checked conversion: elements*element_width must fit SIZE_MAX and available
 * bytes. Output is unchanged on rejection. Width0 is invalid. Distinct wrapper
 * argument types detect accidental byte/element swaps at translation time.
 */
LC_API LCStatus lc_extent(LCElements elements, size_t element_width,
                          LCBytes available, LCBytes *needed);
/** Paired release. Null is a no-op. Nonnull must be live and from this library;
 * stale/arbitrary/cross-allocator pointers violate the precondition.
 */
LC_API void lc_release_buffer(LCBuffer *);
/** Synchronous copied-input example: callback occurs before return, no
 * retention. */
typedef void (*LCDone)(void *, LCStatus);
LC_API LCStatus lc_sync(LCDone, void *borrowed_context);
typedef struct LCOperation LCOperation;
/** Deferred replacement. One owned operation retains callback/context until
 * poll or cancellation acknowledgment. Callback must not reenter this
 * operation. A successful cancel request is not quiescence. Release is LC_BUSY
 * before terminal poll/ack; terminal release owns only operation, never
 * context.
 */
LC_API LCOperation *lc_start(LCDone, void *borrowed_context);
LC_API LCStatus lc_cancel(LCOperation *);
LC_API LCStatus lc_poll(LCOperation *);
LC_API LCStatus lc_ack(LCOperation *);
LC_API LCStatus lc_release_operation(LCOperation *);
#ifdef __cplusplus
}
#endif
#endif
