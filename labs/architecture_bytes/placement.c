#include "bytes.h"
/* Original typed, aligned, CPU-accessible objects. No hardware DMA is run. */
__attribute__((section(".ccm"),used)) struct aw_record ab_cpu_only[2];
__attribute__((section(".sram"),used)) struct aw_record ab_dma_candidate[2];
