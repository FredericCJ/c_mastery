#ifndef CM_PORT_ENCODING_H
#define CM_PORT_ENCODING_H
#include <stdint.h>
int pe_branch(int32_t, unsigned, unsigned, uint32_t *);
int pe_jump(int32_t, unsigned, uint32_t *);
#endif
