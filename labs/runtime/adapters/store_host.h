#ifndef CVM_STORE_HOST_H
#define CVM_STORE_HOST_H
#include <stdint.h>
/* ctx is an opened, exclusively owned store*. Service1 commits vm-result;
   service2 reads it. Adapter failure never means the write definitely did not
   commit. */
int cvm_store_host(void *ctx, uint8_t service, int32_t input, int32_t *output);
#endif
