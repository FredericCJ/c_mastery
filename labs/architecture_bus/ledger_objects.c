#include <stdint.h>
/* Original source object; the destination and CPU-local records are reused
   unchanged from labs/architecture_bytes/placement.c. */
__attribute__((section(".dma_source"), used)) unsigned char mb_dma_source[16];
