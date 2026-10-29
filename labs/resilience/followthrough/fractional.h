#ifndef CMASTERY_RESILIENCE_FRACTIONAL_H
#define CMASTERY_RESILIENCE_FRACTIONAL_H
#include "resilience.h"

/* Credit = tokens + remainder / denominator; full credit has remainder zero. */
struct rzf_bucket {
    uint64_t capacity, numerator, denominator, tokens, remainder, last;
};
struct rzf_credit {
    uint64_t capacity, numerator, denominator, tokens, remainder;
};
enum rzf_downtime { RZF_FREEZE_DOWNTIME, RZF_REFILL_DOWNTIME };

int rzf_bucket_init(struct rzf_bucket *, uint64_t, uint64_t, uint64_t, uint64_t);
int rzf_bucket_consume(struct rzf_bucket *, uint64_t, uint64_t);
int rzf_bucket_pair(struct rzf_bucket *, struct rzf_bucket *, uint64_t, uint64_t);
int rzf_credit_export(const struct rzf_bucket *, uint64_t, struct rzf_credit *);
int rzf_credit_import(const struct rzf_credit *, uint64_t, uint64_t, enum rzf_downtime, struct rzf_bucket *);
#endif
