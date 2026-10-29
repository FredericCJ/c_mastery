#ifndef CMASTERY_CONSTRAINED_ANALYSIS_H
#define CMASTERY_CONSTRAINED_ANALYSIS_H
#include "response.h"
#include <stddef.h>
#include <stdint.h>

enum ca_status
{
    CA_OK,
    CA_UNSUPPORTED,
    CA_INVALID,
    CA_DIFFERENT,
    CA_UNIMPLEMENTED
};

/* Every bit is an externally justified premise, never inferred from a trace. */
enum ca_premise
{
    CA_ONE_CPU = 1,
    CA_FIXED_PREEMPTIVE = 2,
    CA_COST_BOUNDS = 4,
    CA_NO_SUSPENSION = 8,
    CA_INTERRUPTS_CHARGED = 16,
    CA_DRIVERS_BOUNDED = 32,
    CA_BLOCKING_JUSTIFIED = 64,
    CA_PRIORITY_ORDER = 128
};

#define CA_PREMISES 255u

enum ca_arrival
{
    CA_PERIODIC,
    CA_SPORADIC_MINIMUM
};

struct ca_model
{
    unsigned premises;
    enum ca_arrival arrivals;
};
/* Live disjoint storage. Refusals preserve both outputs. An admitted call
 * returns the actual imported analyzer status separately from model admission. */
enum ca_status ca_analyze(const struct ca_model *, const struct rta_task *,
                          unsigned, unsigned, unsigned, struct rta_result *,
                          enum rta_status *);

struct ca_config
{
    unsigned char image_sha256[32];
    uint32_t clock_hz, interrupt_policy, cache_policy, probe_policy;
    uint64_t resolution_ns;
};

enum ca_kind
{
    CA_REQUIREMENT,
    CA_MODEL_BOUND,
    CA_OBSERVED_MEAN,
    CA_OBSERVED_MAX
};

struct ca_evidence
{
    enum ca_kind kind;
    uint64_t numerator, denominator;
    struct ca_config config;
};
/* Exact identity comparison is a configuration gate, not authentication. */
enum ca_status ca_compatible(const struct ca_config *,
                             const struct ca_config *);
enum ca_status ca_evidence(const struct ca_config *, const struct ca_evidence *,
                           enum ca_kind, uint64_t *numerator,
                           uint64_t *denominator);
#endif
