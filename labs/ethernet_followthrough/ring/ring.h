#ifndef ETHERNET_FOLLOWTHROUGH_RING_H
#define ETHERNET_FOLLOWTHROUGH_RING_H
#include "phy.h"
#include <stddef.h>

enum
{
    EN_BOUND,
    EN_RESETTING,
    EN_STOPPED,
    EN_RUNNING,
    EN_STOPPING,
    EN_FAULT
};

enum
{
    EN_RX_COUNT = 4,
    EN_TX_COUNT = 2,
    EN_BUFFER = 2048
};

struct en_descriptor
{
    volatile uint32_t word[4];
};

struct en_memory
{
    struct en_descriptor rx[EN_RX_COUNT], tx[EN_TX_COUNT];
    _Alignas(4) unsigned char rx_bytes[EN_RX_COUNT][EN_BUFFER];
    _Alignas(4) unsigned char tx_bytes[EN_TX_COUNT][EN_BUFFER];
};

_Static_assert(sizeof(struct en_descriptor) == 16, "normal descriptor");
_Static_assert(sizeof(struct en_memory) == 12384,
               "selected normal ring and buffers");

struct en_bus
{
    void *context;
    int (*read)(void *, unsigned offset, uint32_t *);
    int (*write)(void *, unsigned offset, uint32_t);
    void (*barrier)(void *);
    /* Complete the selected-silicon minimum same-register write spacing.
     * Nonzero is failure: a mere register readback is not this guarantee. */
    int (*settle)(void *);
};
struct en_ring;

struct en_ticket
{
    const struct en_ring *owner;
    uint32_t epoch, serial;
    unsigned slot;
};

struct en_frame
{
    const unsigned char *bytes;
    size_t length;
    struct en_ticket ticket;
};

struct en_ring
{
    struct en_memory *memory;
    struct en_bus bus;
    struct ep_phy *phy;
    struct ep_link link;
    uint32_t base, epoch, serial, start, last, deadline, remaining, raw_status;
    unsigned initialized, phase, rx_next, tx_next, dropping, reset_pending;
    unsigned rx_held[EN_RX_COUNT], tx_phase[EN_TX_COUNT];
    uint32_t rx_serial[EN_RX_COUNT], tx_serial[EN_TX_COUNT];
    uint16_t tx_length[EN_TX_COUNT];
    unsigned char mac[6];
    uint32_t accepted, dropped, released, transmitted, failed;
};

int en_range(uint32_t base, size_t length);
int en_init(struct en_ring *, struct en_memory *, uint32_t device_base,
            const struct en_bus *, struct ep_phy *, const unsigned char mac[6]);
int en_reset_begin(struct en_ring *, uint32_t now, unsigned polls,
                   uint32_t timeout);
int en_reset_step(struct en_ring *, uint32_t now);
int en_start(struct en_ring *);
/* Copy-admission only. Success owns a complete internal copy; caller retains
 * no borrow obligation. It does not mean publication, completion or delivery. */
int en_tx_copy(struct en_ring *, const void *, size_t, struct en_ticket *);
int en_tx_publish(struct en_ring *, struct en_ticket);
int en_tx_reap(struct en_ring *, struct en_ticket, uint32_t *raw_status);
int en_rx_take(struct en_ring *, struct en_frame *);
int en_rx_release(struct en_ring *, struct en_ticket);
int en_status(struct en_ring *, uint32_t *snapshot);
int en_stop_begin(struct en_ring *, uint32_t now, unsigned polls,
                  uint32_t timeout);
int en_stop_step(struct en_ring *, uint32_t now);
#endif
