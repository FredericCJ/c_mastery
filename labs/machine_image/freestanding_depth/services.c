/* Learner scaffold. Implement the public contracts without heap or libc. */
#include "services.h"

enum fr_status fr_service_init(struct fr_service *s, fr_output f, void *p)
{
    (void)s;
    (void)f;
    (void)p;
    return FR_UNSUPPORTED;
}

enum fr_status fr_service_ready(struct fr_service *s)
{
    (void)s;
    return FR_UNSUPPORTED;
}

enum fr_status fr_service_begin(struct fr_service *s, const unsigned char *b,
                                size_t n)
{
    (void)s;
    (void)b;
    (void)n;
    return FR_UNSUPPORTED;
}

enum fr_status fr_service_pump(struct fr_service *s)
{
    (void)s;
    return FR_UNSUPPORTED;
}

enum fr_status fr_heap_extend(struct fr_service *s, size_t n, void **p)
{
    (void)s;
    (void)n;
    (void)p;
    return FR_UNSUPPORTED;
}

enum fr_status fr_allocate(struct fr_arena *a, size_t n, size_t align, void **p)
{
    (void)a;
    (void)n;
    (void)align;
    (void)p;
    return FR_UNSUPPORTED;
}

enum fr_status fr_startup(unsigned char *r, size_t n, const unsigned char *rom,
                          size_t m, const struct fr_start_plan *p, size_t cut)
{
    (void)r;
    (void)n;
    (void)rom;
    (void)m;
    (void)p;
    (void)cut;
    return FR_UNSUPPORTED;
}

enum fr_status fr_task_create(struct fr_tasks *t, unsigned v,
                              struct fr_handle *h)
{
    (void)t;
    (void)v;
    (void)h;
    return FR_UNSUPPORTED;
}

enum fr_status fr_task_hook(struct fr_tasks *t, struct fr_handle h,
                            const struct fr_service *s)
{
    (void)t;
    (void)h;
    (void)s;
    return FR_UNSUPPORTED;
}

enum fr_status fr_task_read(const struct fr_tasks *t, struct fr_handle h,
                            unsigned *v)
{
    (void)t;
    (void)h;
    (void)v;
    return FR_UNSUPPORTED;
}

enum fr_status fr_task_destroy(struct fr_tasks *t, struct fr_handle h)
{
    (void)t;
    (void)h;
    return FR_UNSUPPORTED;
}
