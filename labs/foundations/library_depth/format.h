#ifndef CM_LIBRARY_FORMAT_H
#define CM_LIBRARY_FORMAT_H
#include <stddef.h>
#include <stdint.h>

enum lf_status
{
    LF_OK,
    LF_ARGUMENT,
    LF_TRUNCATED,
    LF_ENCODING,
    LF_CAPACITY,
    LF_BACKEND,
    LF_NOT_IMPLEMENTED
};

struct lf_result
{
    size_t required, written;
    int terminated, complete;
};

/* snprintf-style callback. ctx stays live/exclusively owned, input disjoint
 * from dest and result. Return required count excluding NUL, or a negative
 * encoding failure. cap0 permits dest=NULL. Never write beyond cap. */
typedef int (*lf_formatter)(void *ctx, char *dest, size_t cap);
enum lf_status lf_prefix(lf_formatter, void *, char *, size_t,
                         struct lf_result *);
/* Query and render at most63 bytes into original local staging. Failure leaves
 * public dest and result unchanged. Backend inputs must stay stable between
 * calls; detect a changed required count or missing terminator before commit.
 */
enum lf_status lf_transaction(lf_formatter, void *, char *, size_t,
                              struct lf_result *);

enum lf_policy
{
    LF_TEXT,
    LF_U32
};

struct lf_value
{
    enum lf_policy tag;

    union
    {
        const char *text;
        uint32_t u32;
    } data;
};
enum lf_status lf_parse_policy(const char *, enum lf_policy *);
enum lf_status lf_typed(enum lf_policy, struct lf_value, char *, size_t,
                        struct lf_result *);
#endif
