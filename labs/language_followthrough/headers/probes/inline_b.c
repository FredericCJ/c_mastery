#include "experiment.h"
unsigned hh_experiment_b(unsigned x) { return hh_experiment(x); }
#if !HH_CALL_ONLY
hh_experiment_fn hh_experiment_address_b(void) { return hh_experiment; }
#endif
