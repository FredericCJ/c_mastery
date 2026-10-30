/*
 * Integer contracts: checked arithmetic, division conventions, size
 * conversion and a portable big-endian cell codec.
 *
 * Common rules for every function below:
 *  - No allocation, no blocking, no retained pointers: inputs are borrowed for
 *    one call.
 *  - Output pointers must be non-NULL, live and writable. Paired outputs must
 *    designate distinct objects; a NULL or aliased pair returns FI_INVALID.
 *  - Failure atomicity: on any return other than FI_OK the caller's outputs
 *    are unchanged.
 *  - Status codes: FI_INVALID for a bad argument or enumeration value,
 *    FI_RANGE when an operand or the exact result is outside the stated
 *    range, FI_UNSUPPORTED when the platform model does not fit,
 *    FI_UNFINISHED only from the learner scaffold.
 */
#ifndef FI_INTEGER_H
#define FI_INTEGER_H
#include <stddef.h>
#include <stdint.h>

enum fi_status
{
    FI_OK,
    FI_INVALID,
    FI_RANGE,
    FI_UNSUPPORTED,
    FI_UNFINISHED
};

/* Closed interval of int values, inclusive of both ends. Valid only when it
 * contains zero: low <= 0 <= high. It restricts int; it is not a new type. */
struct fi_range
{
    int low;
    int high;
};

enum fi_operation
{
    FI_ADD,
    FI_SUBTRACT,
    FI_MULTIPLY
};

/* How the quotient is rounded; the remainder keeps a == q * b + r.
 * FI_TRUNCATE: toward zero (C's / and %). FI_FLOOR: toward negative infinity,
 * remainder takes the divisor's sign. FI_EUCLIDEAN: remainder is never
 * negative. */
enum fi_division
{
    FI_TRUNCATE,
    FI_FLOOR,
    FI_EUCLIDEAN
};

/* Exact a (op) b. Both operands must lie in range, or FI_RANGE. FI_RANGE also
 * when the exact result leaves the range. No wider type may be used. */
enum fi_status fi_binary(struct fi_range range, enum fi_operation op, int a,
                         int b, int *out);

/* Like fi_binary, but a result outside the range is clipped to the nearest
 * endpoint and returns FI_OK with *clipped = 1. *clipped is 0 when the exact
 * result fits. Operands outside the range still return FI_RANGE. Both outputs
 * are published together or not at all. */
enum fi_status fi_saturate(struct fi_range range, enum fi_operation op, int a,
                           int b, int *out, unsigned *clipped);

/* Quotient and remainder of a / b under the chosen convention. FI_RANGE for a
 * zero divisor, for the unrepresentable quotient "most negative / -1", or when
 * the quotient or remainder leaves the range. */
enum fi_status fi_divide(struct fi_range range, enum fi_division policy, int a,
                         int b, int *quotient, int *remainder);

/* a modulo modulus, always in [0, modulus). FI_INVALID if modulus <= 0. */
enum fi_status fi_mod_positive(int a, int modulus, int *out);

/* Convert a signed value to size_t. FI_RANGE if value < 0 or value > ceiling.
 * The sign is rejected before any conversion to an unsigned type. */
enum fi_status fi_to_size(intmax_t value, size_t ceiling, size_t *out);

/* prefix + count * width in bytes, never exceeding limit. FI_RANGE if the
 * sum would exceed limit or wrap; width 0 yields prefix (which must be
 * <= limit). */
enum fi_status fi_extent(size_t prefix, size_t count, size_t width,
                         size_t limit, size_t *out);

/* fi_extent with a signed element count: a negative count is FI_RANGE. */
enum fi_status fi_layout(size_t prefix, intmax_t count, size_t width,
                         size_t limit, size_t *out);

/* Offset of a logical position inside a window of extent bytes that starts at
 * logical_base. Positions are plain numbers, never pointers. FI_RANGE when
 * position is before the base or more than extent bytes past it. */
enum fi_status fi_window(uintmax_t logical_base, uintmax_t position,
                         size_t extent, size_t *offset);

/* One logical octet per unsigned storage element; no CHAR_BIT==8 requirement.
 * Cell 0 holds the most significant octet. fi_encode_cells returns FI_RANGE if
 * wire needs more than 32 bits; fi_decode_cells returns FI_RANGE if any cell
 * exceeds 255. */
enum fi_status fi_encode_cells(uint_least32_t wire, unsigned int out[4]);
enum fi_status fi_decode_cells(const unsigned int cells[4],
                               uint_least32_t *out);

/* Big-endian wire octets. Returns FI_UNSUPPORTED when CHAR_BIT != 8. */
enum fi_status fi_encode_octets(uint_least32_t wire, unsigned char out[4]);
#endif
