/* Implement the positive constant-capacity proof and masked ring here. */
#define MPC_JOIN_INNER(a, b) a##b
#define MPC_JOIN(a, b) MPC_JOIN_INNER(a, b)

int MPC_JOIN(MPC_PREFIX, _push)(MPC_RING *r, uint32_t value)
{
    (void)r;
    (void)value;
    return MPC_INVALID;
}

int MPC_JOIN(MPC_PREFIX, _pop)(MPC_RING *r, uint32_t *out)
{
    (void)r;
    (void)out;
    return MPC_INVALID;
}

#undef MPC_JOIN
#undef MPC_JOIN_INNER
#undef MPC_PREFIX
#undef MPC_RING
#undef MPC_CAPACITY
