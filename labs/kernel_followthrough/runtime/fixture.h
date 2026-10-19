#ifndef KF_RUNTIME_FIXTURE_H
#define KF_RUNTIME_FIXTURE_H
#include "cvm.h"
static const uint8_t kf_vm_bytes[] = {'C', 'V', 'M',     '1', 1,      0,  0,
                                      8,   0,   0,       0,   OP_INT, 41, 0,
                                      0,   0,   OP_HOST, 1,   OP_RET};
#endif
