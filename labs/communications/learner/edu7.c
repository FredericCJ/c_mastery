#include "com_edu7.h"

/* Buildable exercises: every operation still needs implementation. */
uint16_t edu_crc(const void *v, size_t n)
{
    return 0;
}

int edu_frame_encode(const struct edu_frame *f, void *v, size_t cap,
                     size_t *length)
{
    return -1;
}

int edu_frame_byte(struct edu_framer *p, uint8_t c, int fault,
                   struct edu_frame *out)
{
    return -1;
}

int edu_network_encode(const struct edu_datagram *d, void *v, size_t *n)
{
    return -1;
}

int edu_network_decode(const void *v, size_t n, struct edu_datagram *d)
{
    return -1;
}

int edu_route(const struct edu_datagram *in, unsigned ingress,
              struct edu_datagram *out, unsigned *egress, uint8_t *next)
{
    return -1;
}

int edu_fragment_encode(const struct edu_fragment *f, void *v, size_t *n)
{
    return -1;
}

int edu_fragment_decode(const void *v, size_t n, struct edu_fragment *f)
{
    return -1;
}

int edu_tx_begin(struct edu_tx *t, uint16_t id, uint8_t src, uint8_t dst,
                 const void *v, size_t n, uint64_t now)
{
    return -1;
}

int edu_tx_poll(struct edu_tx *t, uint64_t now, struct edu_fragment *out)
{
    return -1;
}

int edu_tx_ack(struct edu_tx *t, const struct edu_fragment *a, uint64_t now)
{
    return -1;
}

int edu_rx_accept(struct edu_rx *r, const struct edu_fragment *f,
                  struct edu_fragment *ack)
{
    return -1;
}

void edu_rx_release(struct edu_rx *r)
{
    return;
}

int edu_record_decode(const void *v, size_t n, struct edu_record *out)
{
    return -1;
}

int edu_record_encode(const struct edu_record *r, void *v, size_t *n)
{
    return -1;
}

int edu_application(const struct edu_record *r, int16_t *set,
                    struct edu_record *reply)
{
    return -1;
}

int edu_session_receive(struct edu_session *s, uint8_t sender, const void *v,
                        size_t n, const uint8_t **data, size_t *len)
{
    return -1;
}

int edu_session_token_begin(struct edu_session *s, uint8_t sender)
{
    return -1;
}

void edu_session_token_complete(struct edu_session *s, uint8_t holder, int ok)
{
    return;
}

int edu_phy_begin(struct edu_physical *p, uint64_t now)
{
    return -1;
}

int edu_phy_started(struct edu_physical *p, uint64_t now)
{
    return -1;
}

int edu_phy_complete(struct edu_physical *p, uint64_t now, int tc)
{
    return -1;
}

int edu_phy_ready(struct edu_physical *p, uint64_t now)
{
    return -1;
}
