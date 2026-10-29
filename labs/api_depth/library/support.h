#ifndef LC_SUPPORT_H
#define LC_SUPPORT_H
#include <stddef.h>

struct lc_ops
{
    int (*acquire)(void *, unsigned);
    void (*release)(void *, unsigned);
    void *context;
};

struct lc_pair
{
    unsigned mask;
    struct lc_ops ops;
};

int lc_construct(struct lc_pair *, struct lc_ops, int optional);
int lc_construct_nested(struct lc_pair *, struct lc_ops, int optional);
int lc_construct_helpers(struct lc_pair *, struct lc_ops, int optional);
void lc_destroy(struct lc_pair *);

struct lc_backend
{
    unsigned calls;
    int bias;
};

int lc_host_backend(struct lc_backend *, int);
int lc_board_backend(struct lc_backend *, int);

struct lc_backend_ops
{
    int (*apply)(struct lc_backend *, int);
    struct lc_backend *context;
};

int lc_backend_run(struct lc_backend_ops, int);
#endif
