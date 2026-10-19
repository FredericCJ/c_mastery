#ifndef SF_WORKLOAD_H
#define SF_WORKLOAD_H
#include "amp.h"
typedef struct {AfRing ring;_Atomic uint32_t phase;uint32_t offered,admitted,rejected,completed,uncertain,checksum;uint8_t state[128];} SfWork;
void sfw_init(SfWork *);
int sfw_offer(SfWork *,unsigned);
int sfw_work(SfWork *);
int sfw_quiescent(SfWork *,unsigned,unsigned);
int sfw_check(const SfWork *);
int sfw_run(SfWork *,unsigned);
#endif
