#ifndef CMASTERY_PMU_INTEGRATIONS_H
#define CMASTERY_PMU_INTEGRATIONS_H
#include <stddef.h>
#include <stdint.h>
/* Supplied workload adapters. Full lifecycle included; never learner
 * solutions. Both return UINT64_MAX on failure.
 *
 * pf_service_work exists only in builds that supply the corpus events, timer
 * collection and timer components (-DPF_WITH_SERVICE); see pmu_workloads.h. */
uint64_t pf_concurrency_work(size_t, unsigned variant, unsigned repetitions);
#if defined(PF_WITH_SERVICE)
uint64_t pf_service_work(size_t, unsigned variant, unsigned repetitions);
#endif
#endif
