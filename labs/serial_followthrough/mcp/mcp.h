#ifndef SERIAL_MCP_FOLLOWTHROUGH_H
#define SERIAL_MCP_FOLLOWTHROUGH_H
#include <stddef.h>
#include <stdint.h>

enum
{
    MC_OK,
    MC_INVALID,
    MC_IO,
    MC_CRC,
    MC_TIMEOUT,
    MC_DEVICE,
    MC_BUSY,
    MC_EMPTY,
    MC_STALE,
    MC_EXHAUSTED,
    MC_FAILED,
    MC_UNIMPLEMENTED
};

enum
{
    MC_NEW,
    MC_READY,
    MC_POISONED
};

struct mc_bus
{
    void *context;
    /* Bounded, exclusive CS. Success releases CS. ALL returns release the passed
       byte borrows; errors may quarantine adapter-owned staging/CS until reset
       acknowledgement. A nonzero result does not promise device rollback. */
    int (*transfer)(void *, const uint8_t *, uint8_t *, size_t);
    uint32_t (*now)(void *);
};

struct mc_config
{
    unsigned clock_hz, spi_hz, payload_code, rx_depth, loopback, data_rate;
    unsigned
        expected_devid; /* exact selected ID/revision byte, not authentication */
    uint16_t accepted_id;
};

struct mc_frame
{
    uint16_t id;
    uint8_t dlc, brs, esi, data[64];
};

struct mc_driver
{
    unsigned state, payload, rx_depth, pending, terminal, result,
        cancel_requested;
    uint32_t last_token, token;
    struct mc_config config;
};

struct mc_layout
{
    unsigned tx_bytes, rx_bytes, unused_bytes, total;
    uint32_t txcon, rxcon;
};

uint16_t mc_crc(const uint8_t *, size_t);
int mc_layout(unsigned payload_code, unsigned rx_depth, struct mc_layout *);
int mc_init(struct mc_driver *, struct mc_bus, const struct mc_config *,
            unsigned budget);
int mc_send(struct mc_driver *, struct mc_bus, uint32_t token,
            const struct mc_frame *);
int mc_poll(struct mc_driver *, struct mc_bus);
int mc_cancel(struct mc_driver *, struct mc_bus);
int mc_take(struct mc_driver *, struct mc_bus, uint32_t token, unsigned budget,
            unsigned *result);
int mc_receive(struct mc_driver *, struct mc_bus, struct mc_frame *);
int mc_interrupt(struct mc_driver *, struct mc_bus, uint32_t *observed);
int mc_read(struct mc_bus, uint16_t, uint8_t *, unsigned);
int mc_write(struct mc_bus, uint16_t, const uint8_t *, unsigned);
/* No bus action; exact deterministic bench oracle (sequence in ID permits
   the required zero-byte payload). Returned ESI is observational, not payload.
 */
int mc_bench_frame(unsigned index, unsigned brs, struct mc_frame *);
int mc_bench_match(unsigned index, unsigned brs, const struct mc_frame *);
int mc_test_layout(void);
int mc_test_init(void);
int mc_test_failure(void);
int mc_test_transfer(void);
int mc_test_receive(void);
int mc_test_bench(void);
int mc_test_spi(void);
int mc_test_atomic(void);
void mc_values(const char *, unsigned, unsigned, unsigned, unsigned);
void mc_note(const char *);
#endif
