#ifndef COM_MCU_H
#define COM_MCU_H
#include <stddef.h>
#include <stdint.h>

/* One synchronous SPI transfer owns CS from before first bit through final
   completion, releases it on every return; no retained input/output pointers.
 */
struct mcp_bus
{
    void *ctx;
    int (*transfer)(void *, const uint8_t *, uint8_t *, size_t);
    uint64_t (*milliseconds)(void *);
};

int mcp_read32(const struct mcp_bus *, uint16_t, uint32_t *);
int mcp_write32(const struct mcp_bus *, uint16_t, uint32_t);
int mcp_configuration_reset(const struct mcp_bus *, uint64_t timeout_ms);
int can_fd_length(unsigned dlc, unsigned *length);

/* Generic original ownership model, not an STM32 register driver. The board
   adapter must translate observed status and provide the required DMA barriers.
 */
struct dma_slot
{
    uint8_t data[2048];
    size_t length;
    unsigned owner, published, complete;
};

int dma_tx_prepare(struct dma_slot *, const void *, size_t);
int dma_tx_publish(struct dma_slot *, void (*barrier)(void *), void *);
int dma_tx_complete(struct dma_slot *, int error);
int dma_rx_acquire(struct dma_slot *, unsigned own, unsigned fs, unsigned ls,
                   unsigned error, unsigned frame_length, size_t *exposed);
#endif
