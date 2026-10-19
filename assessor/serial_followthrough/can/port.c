#include "can.h"
#include <stdint.h>
#ifdef CN_MAPPED
uint32_t cn_mapped_read(uintptr_t);
void cn_mapped_write(uintptr_t,uint32_t);
#endif
static uint32_t port_read(void *unused,unsigned offset){
 (void)unused;
#ifdef CN_MAPPED
 return cn_mapped_read(UINT32_C(0x40006400)+offset);
#else
 return *(volatile uint32_t *)(uintptr_t)(UINT32_C(0x40006400)+offset);
#endif
}
static void port_write(void *unused,unsigned offset,uint32_t value){
 (void)unused;
#ifdef CN_MAPPED
 cn_mapped_write(UINT32_C(0x40006400)+offset,value);
#else
 *(volatile uint32_t *)(uintptr_t)(UINT32_C(0x40006400)+offset)=value;
#endif
}
CnIO cn_f439_io(void){CnIO io={0,port_read,port_write};return io;}
