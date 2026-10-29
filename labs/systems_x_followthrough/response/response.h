#ifndef SYSTEMS_X_RESPONSE_H
#define SYSTEMS_X_RESPONSE_H
#include "b05.h"
int sx_switch_response(uint32_t own, uint32_t blocking, uint32_t deadline,
                       const struct b5_task *higher, size_t count,
                       uint32_t switch_cost, uint32_t *out);
#endif
