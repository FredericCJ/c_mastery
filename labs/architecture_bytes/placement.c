#include "bytes.h"
/* Original typed, aligned, CPU-accessible objects. No hardware DMA is run. */
__attribute__((section(".ccm"),used)) AWRecord ab_cpu_only[2];
__attribute__((section(".sram"),used)) AWRecord ab_dma_candidate[2];
