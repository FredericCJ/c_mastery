/* Original audit negative: empty nm -u does not imply an allowed provider set. */
#include <stddef.h>
void *malloc(size_t n){(void)n;return 0;}
