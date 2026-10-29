#include "ethernet_depth.h"

/* Implement the named mechanisms from their contracts. */
int ed_wrap(uint32_t application, const uint32_t *headers, size_t count,
            uint32_t *sizes)
{
    (void)application;
    (void)headers;
    (void)count;
    (void)sizes;
    return ED_UNIMPLEMENTED;
}

int ed_compatible(struct ed_interface provided, struct ed_interface required,
                  unsigned *compatible)
{
    (void)provided;
    (void)required;
    (void)compatible;
    return ED_UNIMPLEMENTED;
}

int ed_frame_build(const uint8_t destination[6], const uint8_t source[6],
                   uint16_t field, const uint8_t *payload, size_t length,
                   unsigned fcs, uint8_t *out, size_t capacity, size_t *written)
{
    (void)destination;
    (void)source;
    (void)field;
    (void)payload;
    (void)length;
    (void)fcs;
    (void)out;
    (void)capacity;
    (void)written;
    return ED_UNIMPLEMENTED;
}

int ed_frame_parse(const uint8_t *frame, size_t length, unsigned fcs,
                   struct ed_frame *out)
{
    (void)frame;
    (void)length;
    (void)fcs;
    (void)out;
    return ED_UNIMPLEMENTED;
}

int ed_crc32(const uint8_t *bytes, size_t length, uint32_t *out)
{
    (void)bytes;
    (void)length;
    (void)out;
    return ED_UNIMPLEMENTED;
}

int ed_bridge(struct ed_bridge *bridge, const uint8_t source[6],
              const uint8_t destination[6], unsigned ingress, uint32_t now,
              unsigned *output_mask)
{
    (void)bridge;
    (void)source;
    (void)destination;
    (void)ingress;
    (void)now;
    (void)output_mask;
    return ED_UNIMPLEMENTED;
}

int ed_mdio_word(uint32_t hclk, unsigned phy, unsigned reg, unsigned busy,
                 uint32_t *word, uint32_t *divisor)
{
    (void)hclk;
    (void)phy;
    (void)reg;
    (void)busy;
    (void)word;
    (void)divisor;
    return ED_UNIMPLEMENTED;
}

int ed_mdio_read(const struct ed_bus *bus, uint32_t hclk, unsigned phy,
                 unsigned reg, unsigned budget, uint32_t timeout,
                 uint16_t *value)
{
    (void)bus;
    (void)hclk;
    (void)phy;
    (void)reg;
    (void)budget;
    (void)timeout;
    (void)value;
    return ED_UNIMPLEMENTED;
}

int ed_phy_query(const struct ed_bus *bus, uint32_t hclk, unsigned phy,
                 unsigned budget, uint32_t timeout, struct ed_phy *out)
{
    (void)bus;
    (void)hclk;
    (void)phy;
    (void)budget;
    (void)timeout;
    (void)out;
    return ED_UNIMPLEMENTED;
}

int ed_rx_acquire(struct ed_rx *state, const struct ed_descriptor *descriptors,
                  size_t count, struct ed_borrow *out)
{
    (void)state;
    (void)descriptors;
    (void)count;
    (void)out;
    return ED_UNIMPLEMENTED;
}

int ed_rx_release(struct ed_rx *state, uint32_t token)
{
    (void)state;
    (void)token;
    return ED_UNIMPLEMENTED;
}

int ed_tx_prepare(struct ed_tx *state, unsigned slot, uint32_t token,
                  const uint8_t *frame, size_t length)
{
    (void)state;
    (void)slot;
    (void)token;
    (void)frame;
    (void)length;
    return ED_UNIMPLEMENTED;
}

int ed_tx_tail(struct ed_tx *state, unsigned slot, uint32_t token)
{
    (void)state;
    (void)slot;
    (void)token;
    return ED_UNIMPLEMENTED;
}

int ed_tx_publish(struct ed_tx *state, unsigned slot, uint32_t token,
                  void (*barrier)(void *), void *context)
{
    (void)state;
    (void)slot;
    (void)token;
    (void)barrier;
    (void)context;
    return ED_UNIMPLEMENTED;
}

int ed_tx_complete(struct ed_tx *state, unsigned slot, uint32_t token,
                   unsigned returned, unsigned error)
{
    (void)state;
    (void)slot;
    (void)token;
    (void)returned;
    (void)error;
    return ED_UNIMPLEMENTED;
}

int ed_tx_abort(struct ed_tx *state, unsigned slot, uint32_t token,
                unsigned stopped)
{
    (void)state;
    (void)slot;
    (void)token;
    (void)stopped;
    return ED_UNIMPLEMENTED;
}

int ed_reachable(uint32_t address, uint32_t length)
{
    (void)address;
    (void)length;
    return ED_UNIMPLEMENTED;
}

int ed_initialize(const uint8_t *events, size_t count, size_t *first_bad)
{
    (void)events;
    (void)count;
    (void)first_bad;
    return ED_UNIMPLEMENTED;
}

int ed_budget(uint32_t application, unsigned megabits, struct ed_budget *out)
{
    (void)application;
    (void)megabits;
    (void)out;
    return ED_UNIMPLEMENTED;
}

int ed_capture(const uint32_t *times, size_t count, int32_t offset,
               unsigned offset_known, uint32_t uncertainty, uint64_t *lower,
               uint64_t *upper)
{
    (void)times;
    (void)count;
    (void)offset;
    (void)offset_known;
    (void)uncertainty;
    (void)lower;
    (void)upper;
    return ED_UNIMPLEMENTED;
}

int ed_first_failure(unsigned passed_mask, unsigned *index)
{
    (void)passed_mask;
    (void)index;
    return ED_UNIMPLEMENTED;
}
