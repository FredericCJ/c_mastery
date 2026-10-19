INCLUDE labs/firmware_lifecycle/targets/f439_service.ld
MEMORY { SRAM3 (rw) : ORIGIN = 0x20020000, LENGTH = 64K }
SECTIONS {
 .cpu_local (NOLOAD) : ALIGN(8) { KEEP(*(.ccm)) } > RAM
 .dma_source (NOLOAD) : ALIGN(8) { KEEP(*(.dma_source)) } > RAM
 .dma_destination (NOLOAD) : ALIGN(8) { KEEP(*(.sram)) } > SRAM3
 ASSERT(SIZEOF(.cpu_local)==16 && SIZEOF(.dma_source)==16 && SIZEOF(.dma_destination)==16,"exact live typed storage extent")
 ASSERT(ADDR(.dma_source)+SIZEOF(.dma_source)<=0x20010000,"source stays in selected bank1 service RAM")
 ASSERT(ADDR(.dma_destination)+SIZEOF(.dma_destination)<=0x20030000,"destination stays in SRAM3")
}
