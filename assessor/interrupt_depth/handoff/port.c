#include "handoff.h"
#ifndef XI_PORT_MUTANT
#define XI_PORT_MUTANT 0
#endif
uint32_t xi_primask_enter(void)
{
    uint32_t prior;
    __asm__ volatile("mrs %0,primask\ncpsid i" : "=r"(prior)::"memory");
    return prior;
}

void xi_primask_leave(uint32_t prior)
{
    if (XI_PORT_MUTANT == 1)
    {
        prior = 0;
    }
    __asm__ volatile("msr primask,%0\ndsb\nisb" ::"r"(prior) : "memory");
}

uint32_t xi_basepri_enter(uint32_t threshold)
{
    uint32_t prior;
#if XI_PORT_MUTANT == 2
    __asm__ volatile("mrs %0,basepri\nmsr basepri,%1\ndsb\nisb"
                     : "=&r"(prior)
                     : "r"(threshold)
                     : "memory");
#else
    __asm__ volatile("mrs %0,basepri\nmsr basepri_max,%1\ndsb\nisb"
                     : "=&r"(prior)
                     : "r"(threshold)
                     : "memory");
#endif
    return prior;
}

void xi_basepri_leave(uint32_t prior)
{
    __asm__ volatile("msr basepri,%0\ndsb\nisb" ::"r"(prior) : "memory");
}
