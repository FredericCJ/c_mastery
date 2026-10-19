/* Original narrow Arm EABI glue delegates to the previously validated T11 byte
 * provider. It is not libgcc/compiler-rt. No pointer is touched for size0. */
#include "free.h"
void __aeabi_memcpy(void *d, const void *s, size_t n) {
  (void)fr_copy(d, s, n);
}
void __aeabi_memcpy4(void *d, const void *s, size_t n) {
  (void)fr_copy(d, s, n);
}
void __aeabi_memcpy8(void *d, const void *s, size_t n) {
  (void)fr_copy(d, s, n);
}
void __aeabi_memclr(void *d, size_t n) { (void)fr_set(d, 0, n); }
void __aeabi_memclr4(void *d, size_t n) { (void)fr_set(d, 0, n); }
void __aeabi_memclr8(void *d, size_t n) { (void)fr_set(d, 0, n); }
