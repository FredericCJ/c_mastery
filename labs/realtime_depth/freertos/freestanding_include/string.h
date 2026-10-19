/* Explicit declarations for course-supplied byte helpers, not a hosted libc. */
#ifndef FC_STRING_DECLARATIONS
#define FC_STRING_DECLARATIONS
#include <stddef.h>
void *memcpy(void *restrict, const void *restrict, size_t);
void *memset(void *, int, size_t);
#endif
