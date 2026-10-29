#ifndef COM_EDU7_H
#define COM_EDU7_H
#include <stddef.h>
#include <stdint.h>

struct edu_frame
{
    uint8_t kind, source, destination, length, payload[40];
};

struct edu_framer
{
    uint8_t bytes[47];
    size_t length;
    unsigned escape, discard, active;
};

uint16_t edu_crc(const void *, size_t);
int edu_frame_encode(const struct edu_frame *, void *, size_t, size_t *);
/* 1 complete valid frame, 0 incomplete, -1 drop; UART error invalidates frame.
 */
int edu_frame_byte(struct edu_framer *, uint8_t, int uart_error,
                   struct edu_frame *);

struct edu_datagram
{
    uint8_t source, destination, hop, length, payload[34];
};

int edu_network_encode(const struct edu_datagram *, void *, size_t *);
int edu_network_decode(const void *, size_t, struct edu_datagram *);
/* Router consumes no presentation/session fields; output owns its payload. */
int edu_route(const struct edu_datagram *, unsigned ingress,
              struct edu_datagram *, unsigned *egress, uint8_t *next_link);

struct edu_fragment
{
    uint8_t type, source_port, destination_port, index, count, credit;
    uint16_t id;
    uint8_t data[24];
    size_t length;
};

int edu_fragment_encode(const struct edu_fragment *, void *, size_t *);
int edu_fragment_decode(const void *, size_t, struct edu_fragment *);

struct edu_tx
{
    uint8_t bytes[120];
    size_t length;
    uint16_t id;
    uint8_t source, destination, index, count, credit;
    unsigned active, retries, waiting, blocked;
    uint64_t due, blocked_since;
};

struct edu_rx
{
    uint8_t bytes[120], last[24];
    size_t length, last_length;
    uint16_t id, completed;
    uint8_t source, destination, index, count, last_count;
    unsigned active, ready, publications;
};

int edu_tx_begin(struct edu_tx *, uint16_t, uint8_t, uint8_t, const void *,
                 size_t, uint64_t);
/* 1 emits DATA/PROBE,0 waits,-1 failure; copies immutable bytes each attempt.
 */
int edu_tx_poll(struct edu_tx *, uint64_t, struct edu_fragment *);
int edu_tx_ack(struct edu_tx *, const struct edu_fragment *, uint64_t);
/* ACK output on accepted/duplicate DATA; publication count changes once. */
int edu_rx_accept(struct edu_rx *, const struct edu_fragment *,
                  struct edu_fragment *ack);
void edu_rx_release(struct edu_rx *);

struct edu_record
{
    uint8_t verb;
    uint16_t request;
    int16_t value;
    unsigned has_value, has_unit;
    uint8_t label[16], label_length;
};

int edu_record_decode(const void *, size_t, struct edu_record *);
int edu_record_encode(const struct edu_record *, void *, size_t *);
int edu_application(const struct edu_record *, int16_t *setpoint,
                    struct edu_record *reply);

struct edu_session
{
    uint16_t session, epoch, checkpoint;
    uint8_t holder, last[80];
    size_t last_length;
    unsigned open, token_pending, effects;
};

/* Original L5 bytes: type,sidBE16,epochBE16,checkpointBE16,payload.
   Return 1 new DATA,2 duplicate,0 control,-1 reject. No durable state. */
int edu_session_receive(struct edu_session *, uint8_t sender, const void *,
                        size_t, const uint8_t **data, size_t *length);
int edu_session_token_begin(struct edu_session *, uint8_t sender);
void edu_session_token_complete(struct edu_session *, uint8_t new_holder,
                                int success);

struct edu_physical
{
    unsigned driving, phase;
    uint64_t due;
};

int edu_phy_begin(struct edu_physical *, uint64_t now_us);
int edu_phy_started(struct edu_physical *, uint64_t now_us);
int edu_phy_complete(struct edu_physical *, uint64_t now_us, int uart_tc);
int edu_phy_ready(struct edu_physical *, uint64_t now_us);
#endif
