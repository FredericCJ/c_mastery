#include "edu7_depth.h"

/* Implement the explicitly bounded mechanisms. */
int e7_grant_begin(struct e7_grant *state, uint32_t now, uint32_t *token)
{
    (void)state;
    (void)now;
    (void)token;
    return E7_UNIMPLEMENTED;
}

int e7_grant_first(struct e7_grant *state, uint32_t token, uint32_t now)
{
    (void)state;
    (void)token;
    (void)now;
    return E7_UNIMPLEMENTED;
}

int e7_grant_finish(struct e7_grant *state, uint32_t token, uint32_t now)
{
    (void)state;
    (void)token;
    (void)now;
    return E7_UNIMPLEMENTED;
}

int e7_grant_tick(struct e7_grant *state, uint32_t now)
{
    (void)state;
    (void)now;
    return E7_UNIMPLEMENTED;
}

int e7_link_admit(unsigned kind, unsigned local, unsigned destination,
                  unsigned granted, unsigned transmitting, unsigned uart_fault,
                  unsigned *deliver)
{
    (void)kind;
    (void)local;
    (void)destination;
    (void)granted;
    (void)transmitting;
    (void)uart_fault;
    (void)deliver;
    return E7_UNIMPLEMENTED;
}

int e7_send_begin(struct e7_sender *state, uint16_t id, unsigned source,
                  unsigned destination, const uint8_t *bytes, size_t length,
                  uint32_t now)
{
    (void)state;
    (void)id;
    (void)source;
    (void)destination;
    (void)bytes;
    (void)length;
    (void)now;
    return E7_UNIMPLEMENTED;
}

int e7_send_offer(struct e7_sender *state, uint32_t now,
                  struct e7_fragment *fragment, uint32_t *token)
{
    (void)state;
    (void)now;
    (void)fragment;
    (void)token;
    return E7_UNIMPLEMENTED;
}

int e7_send_accept(struct e7_sender *state, uint32_t token, unsigned accepted,
                   uint32_t now)
{
    (void)state;
    (void)token;
    (void)accepted;
    (void)now;
    return E7_UNIMPLEMENTED;
}

int e7_send_ack(struct e7_sender *state, const struct e7_fragment *ack,
                uint32_t now)
{
    (void)state;
    (void)ack;
    (void)now;
    return E7_UNIMPLEMENTED;
}

int e7_send_expire(struct e7_sender *state, uint32_t now)
{
    (void)state;
    (void)now;
    return E7_UNIMPLEMENTED;
}

int e7_session_open(struct e7_session *state, uint16_t session, uint16_t epoch,
                    unsigned schema, unsigned unit)
{
    (void)state;
    (void)session;
    (void)epoch;
    (void)schema;
    (void)unit;
    return E7_UNIMPLEMENTED;
}

int e7_session_prepare(struct e7_session *state, unsigned sender,
                       uint16_t session, uint16_t epoch, uint16_t checkpoint,
                       const uint8_t *bytes, size_t length, uint32_t *token,
                       uint8_t *cached, size_t capacity, size_t *cached_length)
{
    (void)state;
    (void)sender;
    (void)session;
    (void)epoch;
    (void)checkpoint;
    (void)bytes;
    (void)length;
    (void)token;
    (void)cached;
    (void)capacity;
    (void)cached_length;
    return E7_UNIMPLEMENTED;
}

int e7_session_commit(struct e7_session *state, uint32_t token,
                      unsigned accepted, const uint8_t *response, size_t length)
{
    (void)state;
    (void)token;
    (void)accepted;
    (void)response;
    (void)length;
    return E7_UNIMPLEMENTED;
}

int e7_token_begin(struct e7_session *state, unsigned sender, unsigned next)
{
    (void)state;
    (void)sender;
    (void)next;
    return E7_UNIMPLEMENTED;
}

int e7_token_complete(struct e7_session *state, unsigned next,
                      unsigned confirmed)
{
    (void)state;
    (void)next;
    (void)confirmed;
    return E7_UNIMPLEMENTED;
}

int e7_session_resume(const struct e7_session *state, uint16_t session,
                      uint16_t epoch, unsigned sender, uint16_t checkpoint)
{
    (void)state;
    (void)session;
    (void)epoch;
    (void)sender;
    (void)checkpoint;
    return E7_UNIMPLEMENTED;
}

int e7_session_close(struct e7_session *state)
{
    (void)state;
    return E7_UNIMPLEMENTED;
}
