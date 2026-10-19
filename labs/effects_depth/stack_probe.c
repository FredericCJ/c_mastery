#include <stddef.h>
#include <stdint.h>
#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif
/* Volatile arrays keep concrete stack objects in this compiler experiment.
 * A .su frame includes ABI/compiler overhead; source byte counts are not frames. */
NOINLINE uint32_t ef_stack_callback(uint32_t value){volatile unsigned char bytes[64];for(unsigned i=0;i<64;++i)bytes[i]=(unsigned char)(value+i);return bytes[value%64];}
NOINLINE uint32_t ef_stack_entry(uint32_t value){volatile unsigned char bytes[32];for(unsigned i=0;i<32;++i)bytes[i]=(unsigned char)(value+i);return ef_stack_callback(bytes[value%32])+bytes[0];}
NOINLINE uint32_t ef_stack_recursive(unsigned depth){volatile unsigned char bytes[16];bytes[0]=(unsigned char)depth;if(!depth)return bytes[0];return ef_stack_recursive(depth-1)+bytes[0];}
NOINLINE uint32_t ef_stack_vla(size_t n){if(n==0||n>256)return 0;volatile unsigned char bytes[n];for(size_t i=0;i<n;++i)bytes[i]=(unsigned char)i;return bytes[n-1];}
uint32_t ef_stack_tail(uint32_t value){return ef_stack_callback(value);}
uint32_t ef_stack_indirect(uint32_t(*callback)(uint32_t),uint32_t value){return callback(value);}
int main(void){return ef_stack_entry(2)==10&&ef_stack_recursive(3)==6&&ef_stack_vla(16)==15&&ef_stack_tail(2)==4&&ef_stack_indirect(ef_stack_callback,2)==4?0:1;}
