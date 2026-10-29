#ifndef CMASTERY_EPR_H
#define CMASTERY_EPR_H
#include "errors.h"

/* Zero-initialize fresh storage, then epr_init. Valid controls are exclusively
   owned and unmodified except through this API; inputs/outputs are disjoint. */
struct epr_parser
{
    size_t position, error;
    uint32_t first, second, sum;
    unsigned phase, digits, negative, signed_first;
    er_code terminal;
};

struct epr_step
{
    er_code code;
    size_t consumed, absolute, error;
};

er_code epr_init(struct epr_parser *parser, unsigned signed_first);
struct epr_step epr_feed(struct epr_parser *parser, const unsigned char *bytes,
                         size_t length, unsigned final, uint32_t *sum);
er_result epr_pair_explicit(const unsigned char *bytes, size_t length,
                            uint32_t *sum);
er_result epr_pair_macro(const unsigned char *bytes, size_t length,
                         uint32_t *sum);

static inline er_result epr_identity(er_result result)
{
    return result;
}
#endif
