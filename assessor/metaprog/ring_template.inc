/* Deliberately repeated inclusion: names, element and associated ring type vary. */
#define MP_JOIN_INNER(a,b) a##b
#define MP_JOIN(a,b) MP_JOIN_INNER(a,b)
int MP_JOIN(MP_PREFIX,_push)(MP_RING *ring, MP_ELEMENT value) {
    size_t slot;
    if (ring->count >= 4) return 0;
    slot = MOD_MUTATION == 1 ? ring->count : (ring->head + ring->count) % 4;
    ring->data[slot] = value; ++ring->count; return 1;
}
int MP_JOIN(MP_PREFIX,_pop)(MP_RING *ring, MP_ELEMENT *out) {
    if (ring->count == 0) return 0;
    *out = ring->data[ring->head];
    if (MOD_MUTATION != 2) ring->head = (ring->head + 1) % 4;
    --ring->count; return 1;
}
#undef MP_JOIN
#undef MP_JOIN_INNER
#undef MP_PREFIX
#undef MP_RING
#undef MP_ELEMENT
