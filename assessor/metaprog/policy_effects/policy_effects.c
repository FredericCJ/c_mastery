#include "policy_effects.h"
#ifndef MPP_MUTATION
#define MPP_MUTATION 0
#endif
#define MPP_NAME mpp_dynamic
#define MPP_PARAMETERS size_t count,uint32_t limit,unsigned saturate,MppOps ops,uint32_t *out
#define MPP_SATURATE saturate
#include "policy_template.inc"
#define MPP_NAME mpp_reject
#define MPP_PARAMETERS size_t count,uint32_t limit,MppOps ops,uint32_t *out
#define MPP_SATURATE 0
#include "policy_template.inc"
#define MPP_NAME mpp_saturate
#define MPP_PARAMETERS size_t count,uint32_t limit,MppOps ops,uint32_t *out
#define MPP_SATURATE 1
#include "policy_template.inc"
