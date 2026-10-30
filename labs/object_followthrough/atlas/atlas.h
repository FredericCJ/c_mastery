#ifndef OA_ATLAS_H
#define OA_ATLAS_H
#include <stddef.h>
#include <stdint.h>

/*
 * Contract conventions. Unless a comment says otherwise an int function
 * returns 0 on success and -1 when the request is rejected, and a rejected
 * request leaves every output and owner as it was. Pointer arguments are
 * NULL or point to live objects of the stated type with truthful extents;
 * no function validates an arbitrary address. Nothing here blocks or is
 * thread-safe. The normative text, including the equality policy and the
 * byte encoding, is in CONTRACTS.md.
 */

/* One sample record. Its padding bytes carry no meaning. */
struct oa_record
{
    char marker;
    int32_t number;
    double measure;
};

/* Caller-declared owner of four typed records (a live object, not bytes). */
struct oa_storage
{
    struct oa_record records[4];
};

/* Requests 64-byte alignment, which may exceed _Alignof(max_align_t). */
struct oa_wide
{
    _Alignas(64) double value;
};

/* A borrowed, non-owning view; length counts chars, not terminators. */
struct oa_view
{
    const char *data;
    size_t length;
};

/*
 * Unique owner of a byte allocation. A fresh owner is {NULL, 0, 0};
 * generation counts successful replacements.
 */
struct oa_owner
{
    unsigned char *data;
    size_t length; /* bytes */
    unsigned generation;
};

/* Injected allocation callbacks; context is passed back unchanged. */
struct oa_allocator
{
    void *context;
    void *(*get)(void *context, size_t bytes);
    void (*put)(void *context, void *block);
};

/*
 * Replaces the owner's block by a new block of exactly bytes bytes: the new
 * block is obtained first, the common prefix is copied, any new suffix is
 * zeroed, the old block is returned through put, and generation increases.
 * Rejected for bytes == 0, a saturated generation, an inconsistent owner
 * (NULL data with nonzero length), or when get returns NULL; the owner and
 * its old block are then unchanged (old views remain valid). On success
 * every view of the old block has expired.
 */
int oa_resize(struct oa_owner *owner, size_t bytes,
              const struct oa_allocator *allocator);

/*
 * Returns the owner's block (if any) through put and empties the owner. The
 * generation is kept. Idempotent; ignores NULL arguments. Not an error to
 * call on an empty owner.
 */
void oa_release(struct oa_owner *owner, const struct oa_allocator *allocator);

/*
 * Writes the text "cat" plus its terminator into dest (capacity_bytes must
 * be at least 4) and sets *out to {dest, 3}. The view is valid while dest is.
 */
int oa_label(char *dest, size_t capacity_bytes, struct oa_view *out);

/*
 * Copies exactly source.length chars into dest (no terminator is added) and
 * sets *out to {dest, source.length}. Rejected if source.data is NULL or
 * source.length exceeds capacity_bytes. The copy is independent of source.
 */
int oa_deep_copy(struct oa_view source, char *dest, size_t capacity_bytes,
                 struct oa_view *out);

/*
 * Application equality of two records, defined in CONTRACTS.md. Returns 1
 * when equal and 0 when different or when either pointer is NULL. It never
 * fails otherwise.
 */
int oa_equal(const struct oa_record *left, const struct oa_record *right);

/*
 * Writes the 13-byte canonical encoding of record (defined in CONTRACTS.md)
 * into out. capacity_bytes must be exactly 13: any other size returns -1.
 * Returns -2 when the target lacks the assumed byte or binary64 model. Both
 * failures leave out untouched.
 */
int oa_encode(const struct oa_record *record, unsigned char *out,
              size_t capacity_bytes);

/*
 * Pure arithmetic on a synthetic base known to be aligned: within a region
 * of total_bytes, finds the first offset >= start that is a multiple of
 * alignment_bytes and where width_bytes still fit inside the slice
 * [start, start + length). No pointer is formed. Rejected for a
 * zero or non-power-of-two alignment, a slice outside the region, or when no
 * aligned position fits. On success *out is the offset.
 */
int oa_place(size_t total_bytes, size_t start, size_t length,
             size_t width_bytes, size_t alignment_bytes, size_t *out);

/*
 * Sets *out to the address of the existing record that begins offset_bytes
 * into storage. offset_bytes must be a record boundary, one whole record
 * must be available, and the slice [offset, offset + capacity_bytes) must lie
 * inside storage.
 */
int oa_slot(struct oa_storage *storage, size_t offset_bytes,
            size_t capacity_bytes, struct oa_record **out);

/*
 * Reports (nonzero) whether a caller's stated alignment guarantee, in bytes,
 * is a power of two at least as strict as struct oa_wide needs. The address
 * itself is never inspected.
 */
int oa_wide_admit(size_t guarantee_bytes);

/*
 * Decodes four bytes at any byte offset as a big-endian unsigned value;
 * length_bytes must be at least 4. *out is written only on success.
 */
int oa_decode_u32(const unsigned char *bytes, size_t length_bytes,
                  uint32_t *out);
#endif
