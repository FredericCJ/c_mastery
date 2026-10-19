#ifndef SD_FREESTANDING_STRING_H
#define SD_FREESTANDING_STRING_H
#include <stddef.h>
/* Declarations only. The original byte loops in bytes.c provide the symbols. */
void *memcpy(void *,const void *,size_t);
void *memmove(void *,const void *,size_t);
void *memset(void *,int,size_t);
int memcmp(const void *,const void *,size_t);
#endif
