#ifndef COM_HTTP_H
#define COM_HTTP_H
#include <stddef.h>
#include <stdint.h>

enum
{
    HTTP_NEED = 0,
    HTTP_DONE = 1,
    HTTP_CONTINUE = 2,
    HTTP_ERROR = -1
};

enum
{
    HTTP_LINE,
    HTTP_HEADERS,
    HTTP_BODY,
    HTTP_CHUNK,
    HTTP_CHUNK_DATA,
    HTTP_CHUNK_CR,
    HTTP_CHUNK_LF,
    HTTP_TRAILERS,
    HTTP_FINISHED,
    HTTP_FAILED
};

struct http_request
{
    char method[17], target[1025], authority[128];
    unsigned char body[1024];
    size_t body_length;
    int close, status, head;
};

struct http_parser
{
    struct http_request request;
    char line[8193];
    size_t line_length, header_bytes, trailer_bytes, remaining;
    unsigned fields, trailers, state, saw_cr, host, cl, te, expect, upgrade;
    unsigned te_bad, te_unsupported, te_chunked, te_count;
    int absolute;
    char configured_authority[128];
};

/* Input borrowed only for the call. used reports the exact consumed prefix.
   A DONE/ERROR parser never consumes another request. Reinitialize explicitly.
 */
void http_init(struct http_parser *, const char *authority);
int http_feed(struct http_parser *, const void *, size_t, size_t *used);
int http_eof(struct http_parser *);
/* Inject a trusted IMF-fixdate; it must contain exactly 29 non-control bytes.
 */
int http_response(const struct http_request *, const char *date,
                  int force_close, void *output, size_t capacity,
                  size_t *length);
/* All times are monotonic milliseconds. total does not reset on progress. */
int http_expired(uint64_t now, uint64_t started, uint64_t progress,
                 uint64_t total_limit, uint64_t idle_limit);
/* IO result: positive prefix, -1 retry EINTR, -2 wait EAGAIN, -3 abort.
   At most 16 attempts per call prevents starvation by injected EINTR. */
typedef int (*http_write_fn)(void *, const void *, size_t);
int http_drain(http_write_fn, void *, const void *, size_t, size_t *offset);
#endif
