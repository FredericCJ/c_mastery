#include "atomics.h"

enum
{
    ORDER_COUNT = 6,
    MAX_ATTEMPTS = 16,
    MAX_LEASES = 4
};

/*
 * Whether `failure` is a legal failure order for a compare-exchange whose
 * success order is `success` (both are enum ta_order values). A failure
 * order never releases and is never stronger than the success order.
 * Returns 0 for any value outside the six orders.
 */
int ta_order_pair(unsigned success, unsigned failure)
{
    if (success >= ORDER_COUNT || failure >= ORDER_COUNT)
    {
        return 0;
    }
#ifdef TA_MUTANT_ORDER
    return failure != TA_RELEASE && failure != TA_ACQ_REL;
#else
    /* allowed[s] has bit f set when failure order f is legal with success
       order s; the row index and bit position follow enum ta_order. */
    static const unsigned allowed[ORDER_COUNT] = {1u, 3u, 7u, 1u, 7u, 39u};
    return (allowed[success] & (1u << failure)) != 0;
#endif
}

/*
 * Increment through an injected compare-exchange `operation`. Each attempt
 * recomputes the desired value from the value the failed attempt reported
 * (report.expected), and `weak` is passed as !one_attempt. *out is written on
 * TA_SUCCESS and TA_LIMIT, and untouched on TA_INVALID.
 */
int ta_increment(ta_cas_fn operation, void *context, unsigned initial,
                 unsigned limit, int one_attempt, struct ta_report *out)
{
    if (!operation || !out || limit == 0 || limit > MAX_ATTEMPTS ||
        (one_attempt != 0 && one_attempt != 1))
    {
        return TA_INVALID;
    }
    struct ta_report report = {.expected = initial};
    for (unsigned i = 0; i < (one_attempt ? 1 : limit); i++)
    {
        report.attempts++;
#ifdef TA_MUTANT_RETRY
        report.desired = initial + 1u;
#else
        report.desired = report.expected + 1u;
#endif
        if (operation(context, &report.expected, report.desired, !one_attempt))
        {
            report.published = 1;
            *out = report;
            return TA_SUCCESS;
        }
    }
    *out = report;
    return TA_LIMIT;
}

/* True when `expected` is one of the `count` leased nodes. */
static int is_leased(struct ta_node *const *leases, size_t count,
                     const struct ta_node *expected)
{
    for (size_t j = 0; j < count; j++)
    {
        if (leases[j] == expected)
        {
            return 1;
        }
    }
    return 0;
}

/*
 * Publishes `owned` through an injected node compare-exchange. The caller
 * keeps ownership of `owned` on TA_LIMIT, TA_INVALID and TA_UNPROTECTED; only
 * a successful exchange transfers it. With `derive`, owned->value is
 * recomputed from `expected` on every attempt, and `expected` is dereferenced
 * only if it is one of the leased nodes (otherwise TA_UNPROTECTED).
 */
int ta_publish(ta_node_cas_fn operation, void *context, struct ta_node *owned,
               struct ta_node *expected, struct ta_node *const *leases,
               size_t count, int derive, unsigned limit,
               struct ta_node_report *out)
{
    if (!operation || !owned || !out || !limit || limit > MAX_ATTEMPTS ||
        count > MAX_LEASES || (count && !leases) ||
        (derive != 0 && derive != 1))
    {
        return TA_INVALID;
    }
    struct ta_node_report report = {0};
    for (unsigned i = 0; i < limit; i++)
    {
        if (derive)
        {
            if (!is_leased(leases, count, expected) || !expected)
            {
                *out = report;
                return TA_UNPROTECTED;
            }
            owned->value = expected->value + 1u;
            report.derived_value = owned->value;
        }
        report.attempts++;
        if (operation(context, &expected, owned, 1))
        {
            report.published = 1;
            *out = report;
            return TA_SUCCESS;
        }
#ifdef TA_MUTANT_OWNER
        report.published = 1;
        *out = report;
        return TA_SUCCESS;
#endif
    }
    *out = report;
    return TA_LIMIT;
}
