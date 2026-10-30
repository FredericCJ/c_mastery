/*
 * Sampling loss accounting. Contract and Linux semantics: pmu_loss.h.
 */
#include "pmu_loss.h"

#include <string.h>

enum pf_reading_status pf_sample_reading_decode(const void *bytes,
                                                size_t length, bool with_lost,
                                                struct pf_sample_reading *out)
{
    uint64_t words[4] = {0};
    size_t expected = (with_lost ? 4u : 3u) * sizeof words[0];

    if (length != expected)
    {
        return PF_READING_SHORT;
    }
    /* Copy into typed objects: the caller's buffer need not be aligned. */
    memcpy(words, bytes, expected);
    if (words[2] > words[1])
    {
        return PF_READING_MALFORMED;
    }
    out->value = words[0];
    out->enabled_ns = words[1];
    out->running_ns = words[2];
    out->lost_known = with_lost;
    out->lost = with_lost ? words[3] : 0u;
    return PF_READING_OK;
}

void pf_loss_reconcile(const struct pf_loss_inputs *inputs,
                       struct pf_loss_account *account)
{
    *account = (struct pf_loss_account){
        .accounting = PF_LOSS_UNKNOWN,
        .lower_bound = inputs->inband_lost,
        .hardware_lost = inputs->hardware_lost,
    };
    if (!inputs->kernel_count_known) { account->accounting = PF_LOSS_COMPLETE; account->lossless = inputs->inband_lost == 0u; return; }
    if (inputs->inband_lost > inputs->kernel_lost)
    {
        account->accounting = PF_LOSS_INCONSISTENT;
        return;
    }
    /* The kernel count already includes every loss an in-band record
     * reported; adding the two would double count. */
    account->accounting = PF_LOSS_COMPLETE;
    account->lower_bound = inputs->kernel_lost;
    account->total_lost = inputs->kernel_lost;
    account->unreported_lost = inputs->kernel_lost - inputs->inband_lost;
    account->lossless =
        (inputs->kernel_lost == 0u) && (inputs->hardware_lost == 0u);
}

const char *pf_loss_accounting_name(enum pf_loss_accounting accounting)
{
    switch (accounting)
    {
        case PF_LOSS_COMPLETE:
            return "complete";
        case PF_LOSS_UNKNOWN:
            return "unknown";
        case PF_LOSS_INCONSISTENT:
            return "inconsistent";
        default:
            return "invalid";
    }
}
