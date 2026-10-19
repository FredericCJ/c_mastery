#ifndef FT_LIST_H
#define FT_LIST_H
#include <stddef.h>
#include <stdint.h>
#ifndef FT_CAPACITY
#define FT_CAPACITY 4
#endif
_Static_assert(FT_CAPACITY >= 1 && FT_CAPACITY <= 16,
               "explicit list capacity model");
enum { FT_ERROR_CAPACITY = 8 };
typedef enum {
  FT_OK,
  FT_EMPTY,
  FT_MALFORMED,
  FT_RANGE,
  FT_CAPACITY_ERROR,
  FT_PREFIX,
  FT_INVALID,
  FT_STATE,
  FT_NOT_IMPLEMENTED
} FtStatus;
typedef struct {
  size_t count;
  int32_t sum;
} FtResult;
typedef struct {
  size_t token;
  FtStatus status;
} FtError;
typedef struct {
  FtError items[FT_ERROR_CAPACITY];
  size_t stored, total;
} FtErrors;
typedef struct {
  unsigned state, negative, magnitude, range;
} FtToken;
typedef struct {
  FtToken token;
  int32_t values[FT_CAPACITY];
  size_t count;
  FtStatus status;
  unsigned finished;
} FtStream;
FtStatus ft_tokens(const char *const *tokens, size_t count, int collect,
                   FtResult *out, FtErrors *errors);
void ft_stream_init(FtStream *stream);
FtStatus ft_stream_feed(FtStream *stream, const unsigned char *bytes,
                        size_t count);
FtStatus ft_stream_finish(FtStream *stream, FtResult *out);
const char *ft_name(FtStatus status);
#endif
