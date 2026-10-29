#include "port.h"
#include <stddef.h>
/* Addresses and fields independently checked against pinned ST device header.
   This translation unit is never executed by a hosted register model. */
static uintptr_t address(enum g4_reg r){
 static const uintptr_t addresses[G4_REG_COUNT]={0x40021000u,0x4002104cu,0x40021058u,0x40021038u,0x40021088u,
 0x48000000u,0x48000004u,0x48000008u,0x4800000cu,0x48000020u,
 0x40004400u,0x40004404u,0x40004408u,0x4000440cu,0x4000442cu,0x4000441cu,
 0x40021008u,0x48000024u};
 return addresses[r];
}
static uint32_t read_register(void *ctx,enum g4_reg r){(void)ctx;return *(volatile uint32_t *)address(r);}
static void write_register(void *ctx,enum g4_reg r,uint32_t v){(void)ctx;*(volatile uint32_t *)address(r)=v;}
struct g4_io g4_physical_io(void){struct g4_io io={NULL,read_register,write_register};return io;}
