#include "reducer.h"

enum as_status am_switch(struct am_machine m, enum am_event e,
                         struct am_transition *o)
{
    (void)m;
    (void)e;
    (void)o;
    return AS_UNIMPLEMENTED;
}

enum as_status am_table(struct am_machine m, enum am_event e,
                        struct am_transition *o)
{
    (void)m;
    (void)e;
    (void)o;
    return AS_UNIMPLEMENTED;
}

enum as_status am_functions(struct am_machine m, enum am_event e,
                            struct am_transition *o)
{
    (void)m;
    (void)e;
    (void)o;
    return AS_UNIMPLEMENTED;
}

enum as_status am_sparse(unsigned s, unsigned q, enum am_event e,
                         struct am_transition *o)
{
    (void)s;
    (void)q;
    (void)e;
    (void)o;
    return AS_UNIMPLEMENTED;
}

enum as_status am_execute(struct am_backend *b, struct am_effect e)
{
    (void)b;
    (void)e;
    return AS_UNIMPLEMENTED;
}

enum as_status am_hot_acquire(struct am_hot *h, struct am_lease *l)
{
    (void)h;
    (void)l;
    return AS_UNIMPLEMENTED;
}

enum as_status am_hot_replace(struct am_hot *h, struct am_strategy s)
{
    (void)h;
    (void)s;
    return AS_UNIMPLEMENTED;
}

enum as_status am_hot_release(struct am_hot *h, struct am_lease *l)
{
    (void)h;
    (void)l;
    return AS_UNIMPLEMENTED;
}

enum as_status am_hot_close(struct am_hot *h)
{
    (void)h;
    return AS_UNIMPLEMENTED;
}
