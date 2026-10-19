/* Original compiler-support byte copy. No external memory primitive delegated.
 * This definition closes Clang's aggregate-copy lowering in the core object. */
#include <stddef.h>
void *memcpy(void *restrict destination,const void *restrict source,size_t n){
 volatile unsigned char *d=destination;
 const volatile unsigned char *s=source;
 for(size_t i=0;i<n;++i)d[i]=s[i];
 return destination;
}
