#include "replay_depth.h"

int rpr_encode(const struct rpr_session *a, unsigned char b[512])
{
    (void)a;
    (void)b;
    return RPR_UNIMPLEMENTED;
}

int rpr_decode(uint64_t a, const unsigned char b[512], struct rpr_session *c)
{
    (void)a;
    (void)b;
    (void)c;
    return RPR_UNIMPLEMENTED;
}

int rpr_input_encode(struct rpr_input a, unsigned char b[16])
{
    (void)a;
    (void)b;
    return RPR_UNIMPLEMENTED;
}

int rpr_input_decode(const unsigned char a[16], struct rpr_input *b)
{
    (void)a;
    (void)b;
    return RPR_UNIMPLEMENTED;
}
