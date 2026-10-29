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

struct fi_range
{
    int low, high;
};

enum fi_operation
{
    FI_ADD,
    FI_SUBTRACT,
    FI_MULTIPLY
};

enum fi_division
{
    FI_TRUNCATE,
    FI_FLOOR,
    FI_EUCLIDEAN
};
enum fi_status fi_binary(struct fi_range range, enum fi_operation op, int a,
                         int b, int *out);
enum fi_status fi_saturate(struct fi_range range, enum fi_operation op, int a,
                           int b, int *out, unsigned *clipped);
enum fi_status fi_divide(struct fi_range range, enum fi_division policy, int a,
                         int b, int *quotient, int *remainder);
enum fi_status fi_mod_positive(int a, int modulus, int *out);
enum fi_status fi_to_size(intmax_t value, size_t ceiling, size_t *out);
enum fi_status fi_extent(size_t prefix, size_t count, size_t width,
                         size_t limit, size_t *out);
enum fi_status fi_layout(size_t prefix, intmax_t count, size_t width,
                         size_t limit, size_t *out);
enum fi_status fi_window(uintmax_t logical_base, uintmax_t position,
                         size_t extent, size_t *offset);
/* One logical octet per unsigned storage element; no CHAR_BIT==8 requirement. */
enum fi_status fi_encode_cells(uint_least32_t wire, unsigned int out[4]);
enum fi_status fi_decode_cells(const unsigned int cells[4],
                               uint_least32_t *out);
enum fi_status fi_encode_octets(uint_least32_t wire, unsigned char out[4]);
#endif
