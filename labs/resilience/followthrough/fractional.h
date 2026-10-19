#ifndef CMASTERY_RESILIENCE_FRACTIONAL_H
#define CMASTERY_RESILIENCE_FRACTIONAL_H
#include "resilience.h"

/* Credit = tokens + remainder / denominator; full credit has remainder zero. */
typedef struct {
    uint64_t capacity, numerator, denominator, tokens, remainder, last;
} RzfBucket;
typedef struct {
    uint64_t capacity, numerator, denominator, tokens, remainder;
} RzfCredit;
typedef enum { RZF_FREEZE_DOWNTIME, RZF_REFILL_DOWNTIME } RzfDowntime;

int rzf_bucket_init(RzfBucket *, uint64_t, uint64_t, uint64_t, uint64_t);
int rzf_bucket_consume(RzfBucket *, uint64_t, uint64_t);
int rzf_bucket_pair(RzfBucket *, RzfBucket *, uint64_t, uint64_t);
int rzf_credit_export(const RzfBucket *, uint64_t, RzfCredit *);
int rzf_credit_import(const RzfCredit *, uint64_t, uint64_t, RzfDowntime, RzfBucket *);
#endif
