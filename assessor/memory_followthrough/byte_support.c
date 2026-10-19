#include <stddef.h>
void *memset(void *out,int value,size_t n){unsigned char *p=out;for(size_t i=0;i<n;i++)p[i]=(unsigned char)value;return out;}
void *memcpy(void *out,const void *in,size_t n){unsigned char *d=out;const unsigned char *s=in;for(size_t i=0;i<n;i++)d[i]=s[i];return out;}
