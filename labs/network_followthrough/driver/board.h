#ifndef NB_BOARD_H
#define NB_BOARD_H
#include "driver.h"
#include "ethernet_followthrough/ring/target.h"

enum
{
    NB_BOARD_COLD,
    NB_BOARD_PHY,
    NB_BOARD_RESET,
    NB_BOARD_READY,
    NB_BOARD_FAILED
};

struct nb_board
{
    struct en_target target;
    struct ep_mmio_clock clock;
    struct ep_phy phy;
    struct en_ring ring;
    struct nb_net net;
    struct na_config config;
    struct en_memory *memory;
    uint32_t base, epoch, isn, start, last;
    unsigned phase, capacity;
};

/* Fresh zeroed object. Selected MB1137/F439ZI RMII wiring, stable168MHz HCLK,
 * externally established50MHz RMII reference, exclusive RCC/GPIO/ETH/DWT.
 * Memory must be the actual address 'base' in DMA-visible SRAM. clock is an
 * independently provided monotonic millisecond clock; it must advance, and
 * must not wrap during a session. Requalify before its32-bit horizon. */
int nb_board_begin(struct nb_board *, struct en_memory *, uint32_t base,
                   struct ep_mmio_clock, struct na_config, uint32_t epoch,
                   uint32_t isn, unsigned capacity);
/* At most one PHY/reset transition; call at least once/ms during bring-up.
 * Reset/negotiation jointly bounded500ms and512 steps; each MDIO10ms/1024polls.
 * Result READY means software observations, not independent clock measurement.
 */
int nb_board_step(struct nb_board *);
#endif
