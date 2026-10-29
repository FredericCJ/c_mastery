#ifndef CMASTERY_PMU_INTEGRATIONS_H
#define CMASTERY_PMU_INTEGRATIONS_H
#include <stddef.h>
#include <stdint.h>
/* Supplied workload adapters. Full lifecycle included; never learner solutions. */
uint64_t pf_concurrency_work(size_t, unsigned variant, unsigned repetitions);
uint64_t pf_service_work(size_t, unsigned variant, unsigned repetitions);
#endif
