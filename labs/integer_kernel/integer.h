#ifndef FI_INTEGER_H
#define FI_INTEGER_H
#include <stddef.h>
#include <stdint.h>
typedef enum { FI_OK,FI_INVALID,FI_RANGE,FI_UNSUPPORTED,FI_UNFINISHED } FiStatus;
typedef struct { int low,high; } FiRange;
typedef enum { FI_ADD,FI_SUBTRACT,FI_MULTIPLY } FiOperation;
typedef enum { FI_TRUNCATE,FI_FLOOR,FI_EUCLIDEAN } FiDivision;
FiStatus fi_binary(FiRange range,FiOperation op,int a,int b,int *out);
FiStatus fi_saturate(FiRange range,FiOperation op,int a,int b,int *out,unsigned *clipped);
FiStatus fi_divide(FiRange range,FiDivision policy,int a,int b,int *quotient,int *remainder);
FiStatus fi_mod_positive(int a,int modulus,int *out);
FiStatus fi_to_size(intmax_t value,size_t ceiling,size_t *out);
FiStatus fi_extent(size_t prefix,size_t count,size_t width,size_t limit,size_t *out);
FiStatus fi_layout(size_t prefix,intmax_t count,size_t width,size_t limit,size_t *out);
FiStatus fi_window(uintmax_t logical_base,uintmax_t position,size_t extent,size_t *offset);
/* One logical octet per unsigned storage element; no CHAR_BIT==8 requirement. */
FiStatus fi_encode_cells(uint_least32_t wire,unsigned int out[4]);
FiStatus fi_decode_cells(const unsigned int cells[4],uint_least32_t *out);
FiStatus fi_encode_octets(uint_least32_t wire,unsigned char out[4]);
#endif
