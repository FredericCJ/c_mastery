/* Consumer A of the inline experiment; takes the helper address unless
 * HH_CALL_ONLY. */
#include "experiment.h"

unsigned hh_experiment_a(unsigned x)
{
    return hh_experiment(x);
}
#if !HH_CALL_ONLY
hh_experiment_fn hh_experiment_address_a(void)
{
    return hh_experiment;
}
#endif
