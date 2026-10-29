/* Learner scaffold: implement the contracts in reactor.h. Deliberately builds
 * but rejects work; it is not the assessor reference. */
#include "reactor.h"
#define UNUSED(x) ((void)(x))

int rd_operation_init(struct rd_operation *a, size_t b, const unsigned char *c)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    return -1;
}

int rd_progress(struct rd_operation *a, int b, const unsigned char *c)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    return -1;
}

int rd_cancel(struct rd_operation *a)
{
    UNUSED(a);
    return -1;
}

int rd_registry_init(struct rd_registry *a, unsigned b, unsigned c)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    return -1;
}

int rd_register(struct rd_registry *a, unsigned b, int c, struct rd_token *d)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    UNUSED(d);
    return -1;
}

struct rd_slot *rd_borrow(struct rd_registry *a, struct rd_token b)
{
    UNUSED(a);
    UNUSED(b);
    return 0;
}

int rd_return(struct rd_registry *a, struct rd_token b)
{
    UNUSED(a);
    UNUSED(b);
    return -1;
}

int rd_close(struct rd_registry *a, struct rd_token b)
{
    UNUSED(a);
    UNUSED(b);
    return -1;
}

int rd_submit(struct rd_registry *a, struct rd_token b, unsigned c,
              struct rd_completion *d)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    UNUSED(d);
    return -1;
}

int rd_complete(struct rd_registry *a, struct rd_completion b, int c,
                const unsigned char *d)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    UNUSED(d);
    return -1;
}

int rd_ready(struct rd_scheduler *a, unsigned b)
{
    UNUSED(a);
    UNUSED(b);
    return -1;
}

int rd_turn(struct rd_scheduler *a, unsigned b, int (*c)(void *, unsigned),
            void *d, struct rd_turn *e)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    UNUSED(d);
    UNUSED(e);
    return -1;
}

int rd_service_init(struct rd_service *a, size_t b)
{
    UNUSED(a);
    UNUSED(b);
    return -1;
}

int rd_service_open(struct rd_service *a, unsigned *b)
{
    UNUSED(a);
    UNUSED(b);
    return -1;
}

int rd_admit(struct rd_service *a, unsigned b, struct rf_message *c,
             enum rd_policy d)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    UNUSED(d);
    return -1;
}

int rd_resume(struct rd_service *a, unsigned b)
{
    UNUSED(a);
    UNUSED(b);
    return -1;
}

int rd_commit(struct rd_service *a, unsigned b, size_t c)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    return -1;
}

int rd_service_borrow(struct rd_service *a)
{
    UNUSED(a);
    return -1;
}

int rd_service_return(struct rd_service *a)
{
    UNUSED(a);
    return -1;
}

int rd_stop(struct rd_service *a, uint64_t b, uint64_t c)
{
    UNUSED(a);
    UNUSED(b);
    UNUSED(c);
    return -1;
}

int rd_tick(struct rd_service *a, uint64_t b)
{
    UNUSED(a);
    UNUSED(b);
    return -1;
}
