#ifndef R5_EXPLICIT_BYTE_DECLARATIONS_H
#define R5_EXPLICIT_BYTE_DECLARATIONS_H
#include <stddef.h>
/* Declaration-only interface for the imported framer translation unit.
   Supply and audit the retained routines explicitly; this is not a libc. */
void *memcpy(void *restrict,const void *restrict,size_t);
void *memset(void *,int,size_t);
int memcmp(const void *,const void *,size_t);
#endif
