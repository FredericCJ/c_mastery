/* Explicit target primitive required by the selected provider's generated
 * code. No libc is linked. Volatile byte stores prevent self-recursive memset
 * lowering; they are not synchronization and apply only to ordinary storage. */
#include <stddef.h>
void *memset(void *destination,int value,size_t count){volatile unsigned char*p=destination;for(size_t i=0;i<count;++i)p[i]=(unsigned char)value;return destination;}
