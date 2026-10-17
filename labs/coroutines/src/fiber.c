#include "fiber.h"
int co_fiber_init(COFiber*f,void*s,size_t n,COFiberFn fn,void*a){(void)f;(void)s;(void)n;(void)fn;(void)a;return 9;}
int co_fiber_resume(COFiber*f){(void)f;return 9;}
int co_fiber_yield(COFiber*f){(void)f;return 9;}
int co_fiber_destroy(COFiber*f){(void)f;return 9;}
