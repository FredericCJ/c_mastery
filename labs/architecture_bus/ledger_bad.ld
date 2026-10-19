/* Original deliberately wrong placement for the rejection control; never run. */
INCLUDE labs/firmware_lifecycle/targets/f439_service.ld
MEMORY { CCM (rw) : ORIGIN = 0x10000000, LENGTH = 64K }
SECTIONS {
 .cpu_local (NOLOAD) : ALIGN(8) { KEEP(*(.ccm)) } > RAM
 .dma_source (NOLOAD) : ALIGN(8) { KEEP(*(.dma_source)) } > RAM
 .dma_destination (NOLOAD) : ALIGN(8) { KEEP(*(.sram)) } > CCM
}
