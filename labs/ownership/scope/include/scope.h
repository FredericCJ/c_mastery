#ifndef CMASTERY_OWNERSHIP_SCOPE_DEPTH_H
#define CMASTERY_OWNERSHIP_SCOPE_DEPTH_H
#include "ownership.h"
#include "errors.h"

struct ocs_plan
{
    unsigned count;
    enum ow_when policy[8];
    unsigned fail_before;
    er_code raised;
};

struct ocs_report
{
    er_code original, converted;
    unsigned registered, trace[8], seen_error[8], count;
};
/* Ordinary callbacks are nonfailing. Errors are converted before final cleanup. */
enum ow_status ocs_run(const struct ocs_plan *, unsigned *external_effects,
                       struct ocs_report *);

enum ocs_close
{
    OCS_RELEASED,
    OCS_RETAINED,
    OCS_RELEASED_ERROR
};
typedef enum ocs_close (*ocs_closer_fn)(void *, unsigned identity);

struct ocs_entry
{
    ocs_closer_fn close;
    void *context;
    unsigned identity;
};
struct ocs_recovery;

struct ocs_ticket
{
    struct ocs_recovery *owner;
    uint32_t generation;
};

struct ocs_recovery
{
    struct ocs_entry entry[8];
    unsigned count, active, finishing, sealed, errors;
    uint32_t generation;
};

struct ocs_recovered
{
    unsigned pending, close_errors, recovery_required;
};
enum ow_status ocs_begin(struct ocs_recovery *, struct ocs_ticket *);
enum ow_status ocs_push(struct ocs_ticket, ocs_closer_fn, void *, unsigned);
enum ow_status ocs_recover(struct ocs_ticket, struct ocs_recovered *);
#endif
