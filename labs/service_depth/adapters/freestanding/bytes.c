#include <stddef.h>
#include <stdint.h>
void *memcpy(void *p,const void *q,size_t n){volatile unsigned char *a=p;const volatile unsigned char *b=q;for(size_t i=0;i<n;i++)a[i]=b[i];return p;}
void *memset(void *p,int c,size_t n){volatile unsigned char *a=p;for(size_t i=0;i<n;i++)a[i]=(unsigned char)c;return p;}
int memcmp(const void *p,const void *q,size_t n){const volatile unsigned char *a=p,*b=q;for(size_t i=0;i<n;i++)if(a[i]!=b[i])return a[i]<b[i]?-1:1;return 0;}
void *memmove(void *p,const void *q,size_t n){volatile unsigned char *a=p;const volatile unsigned char *b=q;
  /* Explicit flat-address target contract; not a portable ordering of objects. */
  if((uintptr_t)p<(uintptr_t)q){for(size_t i=0;i<n;i++)a[i]=b[i];}
  else {while(n){n--;a[n]=b[n];}}return p;
}
