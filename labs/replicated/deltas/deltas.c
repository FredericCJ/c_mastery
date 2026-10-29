#include "deltas.h"

int rpd_set_add(const struct rpd_set *state, unsigned writer, unsigned value,
                struct rpd_set *out, struct rpd_set *delta)
{
    (void)state;
    (void)writer;
    (void)value;
    (void)out;
    (void)delta;
    return RPD_UNIMPLEMENTED;
}

int rpd_set_remove(const struct rpd_set *state, unsigned value,
                   struct rpd_set *out, struct rpd_set *delta)
{
    (void)state;
    (void)value;
    (void)out;
    (void)delta;
    return RPD_UNIMPLEMENTED;
}

int rpd_set_join(const struct rpd_set *state, const struct rpd_set *delta,
                 struct rpd_set *out)
{
    (void)state;
    (void)delta;
    (void)out;
    return RPD_UNIMPLEMENTED;
}

int rpd_contains(const struct rpd_set *state, unsigned value)
{
    (void)state;
    (void)value;
    return RPD_UNIMPLEMENTED;
}

int rpd_encode(const struct rpd_state *state, unsigned char out[128])
{
    (void)state;
    (void)out;
    return RPD_UNIMPLEMENTED;
}

int rpd_decode(const unsigned char wire[128], struct rpd_state *out)
{
    (void)wire;
    (void)out;
    return RPD_UNIMPLEMENTED;
}

int rpd_create(store *pages, uint64_t uuid, unsigned self,
               struct rpd_replica *out)
{
    (void)pages;
    (void)uuid;
    (void)self;
    (void)out;
    return RPD_UNIMPLEMENTED;
}

int rpd_open(store *pages, uint64_t uuid, unsigned self,
             struct rpd_replica *out)
{
    (void)pages;
    (void)uuid;
    (void)self;
    (void)out;
    return RPD_UNIMPLEMENTED;
}

int rpd_local(struct rpd_replica *replica, unsigned add, unsigned value)
{
    (void)replica;
    (void)add;
    (void)value;
    return RPD_UNIMPLEMENTED;
}

int rpd_send(struct rpd_replica *replica, unsigned peer, uint64_t peer_epoch,
             struct rpd_packet *out)
{
    (void)replica;
    (void)peer;
    (void)peer_epoch;
    (void)out;
    return RPD_UNIMPLEMENTED;
}

int rpd_receive(struct rpd_replica *replica, const struct rpd_packet *packet,
                struct rpd_ack *out)
{
    (void)replica;
    (void)packet;
    (void)out;
    return RPD_UNIMPLEMENTED;
}

int rpd_ack(struct rpd_replica *replica, uint64_t current_peer_epoch,
            const struct rpd_ack *ack)
{
    (void)replica;
    (void)current_peer_epoch;
    (void)ack;
    return RPD_UNIMPLEMENTED;
}
