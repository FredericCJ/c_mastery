#include "handoff.h"

/* Target learner scaffold; these deliberately do not alter masks. */
uint32_t xi_primask_enter(void)
{
    return UINT32_MAX;
}

void xi_primask_leave(uint32_t p)
{
    (void)p;
}

uint32_t xi_basepri_enter(uint32_t p)
{
    (void)p;
    return UINT32_MAX;
}

void xi_basepri_leave(uint32_t p)
{
    (void)p;
}
