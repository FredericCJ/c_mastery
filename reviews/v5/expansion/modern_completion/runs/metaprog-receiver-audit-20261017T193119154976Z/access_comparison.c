#include <stdint.h>
uint32_t plain_twice(const uint32_t *p){uint32_t a=*p;uint32_t b=*p;return a+b;}
uint32_t volatile_twice(const volatile uint32_t *p){uint32_t a=*p;uint32_t b=*p;return a+b;}
