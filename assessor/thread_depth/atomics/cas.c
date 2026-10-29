#include "atomics.h"

int ta_order_pair(unsigned success, unsigned failure)
{
    if (success >= 6 || failure >= 6)
    {
        return 0;
    }
#ifdef TA_MUTANT_ORDER
    return failure != TA_RELEASE && failure != TA_ACQ_REL;
#else
    static const unsigned allowed[6] = {1u, 3u, 7u, 1u, 7u, 39u};
    return (allowed[success] & (1u << failure)) != 0;
#endif
}

int ta_increment(ta_cas_fn operation, void *context, unsigned initial,
                 unsigned limit, int one_attempt, struct ta_report *out)
{
    if (!operation || !out || limit == 0 || limit > 16 ||
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

int ta_publish(ta_node_cas_fn operation, void *context, struct ta_node *owned,
               struct ta_node *expected, struct ta_node *const *leases,
               size_t count, int derive, unsigned limit,
               struct ta_node_report *out)
{
    if (!operation || !owned || !out || !limit || limit > 16 || count > 4 ||
        (count && !leases) || (derive != 0 && derive != 1))
    {
        return TA_INVALID;
    }
    struct ta_node_report report = {0};
    for (unsigned i = 0; i < limit; i++)
    {
        if (derive)
        {
            int protected = 0;
            for (size_t j = 0; j < count; j++)
            {
                if (leases[j] == expected)
                {
                    protected = 1;
                }
            }
            if (!protected || !expected)
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
