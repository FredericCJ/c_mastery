#ifndef COM_MCU_H
#define COM_MCU_H
#include <stddef.h>
#include <stdint.h>
/* One synchronous SPI transfer owns CS from before first bit through final
   completion, releases it on every return; no retained input/output pointers.
 */
typedef struct {
  void *ctx;
  int (*transfer)(void *, const uint8_t *, uint8_t *, size_t);
  uint64_t (*milliseconds)(void *);
} McpBus;
int mcp_read32(const McpBus *, uint16_t, uint32_t *);
int mcp_write32(const McpBus *, uint16_t, uint32_t);
int mcp_configuration_reset(const McpBus *, uint64_t timeout_ms);
int can_fd_length(unsigned dlc, unsigned *length);
/* Generic original ownership model, not an STM32 register driver. The board
   adapter must translate observed status and provide the required DMA barriers.
 */
typedef struct {
  uint8_t data[2048];
  size_t length;
  unsigned owner, published, complete;
} DmaSlot;
int dma_tx_prepare(DmaSlot *, const void *, size_t);
int dma_tx_publish(DmaSlot *, void (*barrier)(void *), void *);
int dma_tx_complete(DmaSlot *, int error);
int dma_rx_acquire(DmaSlot *, unsigned own, unsigned fs, unsigned ls,
                   unsigned error, unsigned frame_length, size_t *exposed);
#endif
