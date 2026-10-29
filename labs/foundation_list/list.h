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
enum ft_status {
  FT_OK,
  FT_EMPTY,
  FT_MALFORMED,
  FT_RANGE,
  FT_CAPACITY_ERROR,
  FT_PREFIX,
  FT_INVALID,
  FT_STATE,
  FT_NOT_IMPLEMENTED
};
struct ft_result {
  size_t count;
  int32_t sum;
};
struct ft_error {
  size_t token;
  enum ft_status status;
};
struct ft_errors {
  struct ft_error items[FT_ERROR_CAPACITY];
  size_t stored, total;
};
struct ft_token {
  unsigned state, negative, magnitude, range;
};
struct ft_stream {
  struct ft_token token;
  int32_t values[FT_CAPACITY];
  size_t count;
  enum ft_status status;
  unsigned finished;
};
enum ft_status ft_tokens(const char *const *tokens, size_t count, int collect,
                   struct ft_result *out, struct ft_errors *errors);
void ft_stream_init(struct ft_stream *stream);
enum ft_status ft_stream_feed(struct ft_stream *stream, const unsigned char *bytes,
                        size_t count);
enum ft_status ft_stream_finish(struct ft_stream *stream, struct ft_result *out);
const char *ft_name(enum ft_status status);
#endif
