#ifndef CMASTERY_NESTED_VALIDATION_H
#define CMASTERY_NESTED_VALIDATION_H
#include "errors.h"

struct evv_input
{
    size_t count;
    er_config item[3];
    uint32_t limit;
};

struct evv_diagnostic
{
    unsigned code, depth, path[3];
};

struct evv_summary
{
    size_t total, stored;
    unsigned truncated, ratio_checks;
};
/* root!=0; count0..3; cap0..9; ratio is exact rational, not truncated division.
   Live initialized disjoint objects; see VALIDATION.md for ordered phases. */
struct evv_summary evv_validate(const struct evv_input *, unsigned root,
                                struct evv_input *, struct evv_diagnostic *,
                                size_t capacity);
#endif
