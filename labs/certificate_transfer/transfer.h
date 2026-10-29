#ifndef CMASTERY_CERTIFICATE_TRANSFER_H
#define CMASTERY_CERTIFICATE_TRANSFER_H
#include "frontier.h"
#include "tree.h"

enum ct_status
{
    CT_OK,
    CT_MORE,
    CT_INVALID,
    CT_TRUNCATED,
    CT_CERTIFICATE,
    CT_STATE
};

struct ct_stream
{
    unsigned char bytes[102];
    size_t used, expected;
    unsigned state;
};

void ct_init(struct ct_stream *);
enum ct_status ct_feed(struct ct_stream *, const unsigned char *, size_t,
                       unsigned final, const struct ag_certificate *,
                       struct cd_csr *);

struct ct_frame
{
    int node, parent, phase, left_height, right_height, left_black, right_black;
    int64_t low, high;
};

struct ct_workspace
{
    struct ct_frame frames[64];
    unsigned seen[64], high_water;
};

enum
{
    CT_SPACE = 99
};

int ct_tree(const struct at_certificate *, unsigned policy,
            struct ct_workspace *, unsigned capacity, int *);
int ct_tests(unsigned);
int ct_failure(const char *, unsigned);
void ct_observation(const char *, unsigned, unsigned);
#endif
