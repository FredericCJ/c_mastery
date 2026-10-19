#include "fractional.h"

static int valid(const RzfBucket *b) {
    return b && b->capacity && b->capacity <= UINT32_MAX &&
        b->numerator && b->numerator <= UINT16_MAX &&
        b->denominator && b->denominator <= UINT16_MAX &&
        b->tokens <= b->capacity && b->remainder < b->denominator &&
        (b->tokens != b->capacity || b->remainder == 0);
}

int rzf_bucket_init(RzfBucket *out, uint64_t capacity, uint64_t p,
                    uint64_t q, uint64_t now) {
    RzfBucket b = {capacity, p, q, capacity, 0, now};
    if (!out || !valid(&b)) return RZ_INVALID;
    *out = b;
    return RZ_OK;
}

static void refill(RzfBucket *b, uint64_t now) {
    uint64_t elapsed = now - b->last;
    uint64_t needed = (b->capacity - b->tokens) * b->denominator - b->remainder;
    uint64_t saturation = needed / b->numerator + (needed % b->numerator != 0);
    if (elapsed >= saturation) {
        b->tokens = b->capacity;
        b->remainder = 0;
    } else {
        uint64_t credit = b->remainder + elapsed * b->numerator;
        b->tokens += credit / b->denominator;
#if RZF_MUTANT == 21
        b->remainder = 0;
#else
        b->remainder = credit % b->denominator;
#endif
    }
    b->last = now;
}

int rzf_bucket_consume(RzfBucket *b, uint64_t now, uint64_t cost) {
    if (!valid(b)) return RZ_INVALID;
    if (now < b->last || cost > b->capacity) return RZ_RANGE;
    refill(b, now);
    if (cost > b->tokens) return RZ_DENIED;
    b->tokens -= cost;
    return RZ_OK;
}

int rzf_bucket_pair(RzfBucket *average, RzfBucket *peak, uint64_t now, uint64_t cost) {
    if (average == peak || !valid(average) || !valid(peak)) return RZ_INVALID;
    if (now < average->last || now < peak->last ||
        cost > average->capacity || cost > peak->capacity) return RZ_RANGE;
    RzfBucket a = *average, p = *peak;
    refill(&a, now);
    refill(&p, now);
    int status = cost <= a.tokens && cost <= p.tokens ? RZ_OK : RZ_DENIED;
    if (status == RZ_OK) { a.tokens -= cost; p.tokens -= cost; }
#if RZF_MUTANT == 22
    else if (cost <= a.tokens) a.tokens -= cost;
#endif
    *average = a;
    *peak = p;
    return status;
}

int rzf_credit_export(const RzfBucket *b, uint64_t now, RzfCredit *out) {
    if (!out || !valid(b)) return RZ_INVALID;
    if (now < b->last) return RZ_RANGE;
    RzfBucket copy = *b;
    refill(&copy, now);
    *out = (RzfCredit){copy.capacity, copy.numerator, copy.denominator,
                       copy.tokens, copy.remainder};
    return RZ_OK;
}

int rzf_credit_import(const RzfCredit *credit, uint64_t now, uint64_t downtime,
                      RzfDowntime policy, RzfBucket *out) {
    if (!credit || !out || (policy != RZF_FREEZE_DOWNTIME && policy != RZF_REFILL_DOWNTIME))
        return RZ_INVALID;
    RzfBucket b = {credit->capacity, credit->numerator, credit->denominator,
                    credit->tokens, credit->remainder, 0};
    if (!valid(&b)) return RZ_INVALID;
    if (policy == RZF_REFILL_DOWNTIME) refill(&b, downtime);
    b.last = now;
    *out = b;
    return RZ_OK;
}
