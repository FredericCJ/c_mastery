#include "com_mcu.h"

/* Buildable exercises: every operation still needs implementation. */
int mcp_read32(const struct mcp_bus *b, uint16_t address, uint32_t *value)
{
    return -1;
}

int mcp_write32(const struct mcp_bus *b, uint16_t address, uint32_t value)
{
    return -1;
}

int mcp_configuration_reset(const struct mcp_bus *b, uint64_t timeout)
{
    return -1;
}

int can_fd_length(unsigned dlc, unsigned *length)
{
    return -1;
}

int dma_tx_prepare(struct dma_slot *s, const void *v, size_t n)
{
    return -1;
}

int dma_tx_complete(struct dma_slot *s, int error)
{
    return -1;
}

int dma_rx_acquire(struct dma_slot *s, unsigned own, unsigned fs, unsigned ls,
                   unsigned error, unsigned fl, size_t *exposed)
{
    return -1;
}

int dma_tx_publish(struct dma_slot *s, void (*barrier)(void *), void *ctx)
{
    (void)s;
    (void)barrier;
    (void)ctx;
    return -1;
}
