/* Repeated inclusion, constant capacity proof before masked indexing. */
#if !defined(MPC_PREFIX) || !defined(MPC_RING) || !defined(MPC_CAPACITY)
#error MPC_REQUIRED_PARAMETERS
#endif
#if defined(MPC_JOIN) || defined(MPC_JOIN_INNER)
#error MPC_PRIVATE_MACRO_COLLISION
#endif
#define MPC_JOIN_INNER(a, b) a##b
#define MPC_JOIN(a, b) MPC_JOIN_INNER(a, b)
_Static_assert(MPC_CAPACITY > 0 && (MPC_CAPACITY & (MPC_CAPACITY - 1)) == 0,
               "MPC_POWER_OF_TWO");
_Static_assert(sizeof(((MPC_RING *)0)->data) / sizeof(uint32_t) == MPC_CAPACITY,
               "MPC_STORAGE_EXTENT");

int MPC_JOIN(MPC_PREFIX, _push)(MPC_RING *r, uint32_t value)
{
    if (!r)
        return MPC_INVALID;
    if (r->count == MPC_CAPACITY)
        return MPC_FULL;
    r->data[(MPC_MUTATION == 1 ? r->count : r->head + r->count) &
            (MPC_CAPACITY - 1)] = value;
    r->count++;
    return MPC_OK;
}

int MPC_JOIN(MPC_PREFIX, _pop)(MPC_RING *r, uint32_t *out)
{
    if (!r || !out)
        return MPC_INVALID;
    if (!r->count)
        return MPC_EMPTY;
    *out = r->data[r->head];
    if (MPC_MUTATION != 2)
        r->head = (r->head + 1) & (MPC_CAPACITY - 1);
    r->count--;
    return MPC_OK;
}

#undef MPC_JOIN
#undef MPC_JOIN_INNER
#undef MPC_PREFIX
#undef MPC_RING
#undef MPC_CAPACITY
