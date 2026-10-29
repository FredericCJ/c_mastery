#include "commit.h"

int at_same(const struct sc_machine *a, const struct sc_machine *b)
{
    (void)a;
    (void)b;
    return 0;
}

enum at_status at_init(struct at_machine *a, const struct sc_machine *b,
                       unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return AT_INVALID;
}

enum at_status at_input(struct at_machine *a, unsigned b, unsigned c)
{
    (void)a;
    (void)b;
    (void)c;
    return AT_INVALID;
}

enum at_status at_begin(struct at_machine *a, const struct sc_instruction *b,
                        size_t c, struct at_trace *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AT_INVALID;
}

enum at_status at_poll(struct at_machine *a, unsigned b, unsigned c,
                       struct at_trace *d)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    return AT_INVALID;
}

enum at_status at_predict(const struct at_machine *a,
                          const struct sc_instruction *b, size_t c, unsigned d,
                          struct at_spec *e)
{
    (void)a;
    (void)b;
    (void)c;
    (void)d;
    (void)e;
    return AT_INVALID;
}

enum at_status at_resolve(struct at_machine *a, struct at_spec *b,
                          struct at_trace *c)
{
    (void)a;
    (void)b;
    (void)c;
    return AT_INVALID;
}

enum at_status at_retire(struct at_machine *a, struct at_spec *b,
                         struct at_trace *c)
{
    (void)a;
    (void)b;
    (void)c;
    return AT_INVALID;
}

enum at_status at_discard(struct at_spec *a)
{
    (void)a;
    return AT_INVALID;
}
