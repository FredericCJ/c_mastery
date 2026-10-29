#ifndef ETHERNET_DEPTH_H
#define ETHERNET_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum {
  ED_OK = 0,
  ED_INVALID = 1,
  ED_FULL = 2,
  ED_BUSY = 3,
  ED_STALE = 4,
  ED_IO = 5,
  ED_TIMEOUT = 6,
  ED_WRONG_PHY = 7,
  ED_LINK_DOWN = 8,
  ED_NEGOTIATING = 9,
  ED_UNSUPPORTED = 10,
  ED_UNIMPLEMENTED = 99
};
int ed_wrap(uint32_t application, const uint32_t *headers, size_t count,
            uint32_t *sizes);
struct ed_interface {
  uint16_t protocol, version;
  uint8_t shape, ownership;
};
int ed_compatible(struct ed_interface provided, struct ed_interface required,
                  unsigned *compatible);
struct ed_frame {
  uint8_t destination[6], source[6];
  uint16_t field;
  size_t payload_length, stored_payload;
  const uint8_t *payload;
};
int ed_frame_build(const uint8_t destination[6], const uint8_t source[6],
                   uint16_t field, const uint8_t *payload, size_t length,
                   unsigned fcs, uint8_t *out, size_t capacity,
                   size_t *written);
int ed_frame_parse(const uint8_t *frame, size_t length, unsigned fcs,
                   struct ed_frame *out);
int ed_crc32(const uint8_t *bytes, size_t length, uint32_t *out);
struct ed_bridge_entry {
  uint8_t mac[6], port, live;
  uint32_t seen;
};
struct ed_bridge {
  struct ed_bridge_entry entries[4];
  uint32_t now;
};
int ed_bridge(struct ed_bridge *bridge, const uint8_t source[6],
              const uint8_t destination[6], unsigned ingress, uint32_t now,
              unsigned *output_mask);
struct ed_bus {
  void *context;
  int (*read)(void *, unsigned, uint32_t *);
  int (*write)(void *, unsigned, uint32_t);
  uint32_t (*clock)(void *);
};
struct ed_phy {
  uint8_t historical, current, full;
  uint16_t speed;
};
int ed_mdio_word(uint32_t hclk, unsigned phy, unsigned reg, unsigned busy,
                 uint32_t *word, uint32_t *divisor);
int ed_mdio_read(const struct ed_bus *bus, uint32_t hclk, unsigned phy, unsigned reg,
                 unsigned budget, uint32_t timeout, uint16_t *value);
int ed_phy_query(const struct ed_bus *bus, uint32_t hclk, unsigned phy, unsigned budget,
                 uint32_t timeout, struct ed_phy *out);
struct ed_descriptor {
  const uint8_t *bytes;
  uint16_t capacity;
  uint32_t status;
};
struct ed_rx {
  uint32_t generation;
  uint8_t held, dropping;
};
struct ed_borrow {
  const uint8_t *first, *second;
  uint16_t first_length, second_length;
  uint32_t token;
};
int ed_rx_acquire(struct ed_rx *state, const struct ed_descriptor *descriptors, size_t count,
                  struct ed_borrow *out);
int ed_rx_release(struct ed_rx *state, uint32_t token);
struct ed_tx_slot {
  uint8_t bytes[1514], phase;
  uint16_t length;
  uint32_t token, head, tail;
};
struct ed_tx {
  struct ed_tx_slot slots[2];
};
int ed_tx_prepare(struct ed_tx *state, unsigned slot, uint32_t token,
                  const uint8_t *frame, size_t length);
int ed_tx_tail(struct ed_tx *state, unsigned slot, uint32_t token);
int ed_tx_publish(struct ed_tx *state, unsigned slot, uint32_t token,
                  void (*barrier)(void *), void *context);
int ed_tx_complete(struct ed_tx *state, unsigned slot, uint32_t token,
                   unsigned returned, unsigned error);
int ed_tx_abort(struct ed_tx *state, unsigned slot, uint32_t token, unsigned stopped);
int ed_reachable(uint32_t address, uint32_t length);
int ed_initialize(const uint8_t *events, size_t count, size_t *first_bad);
struct ed_budget {
  uint32_t ip, frame, slot, nanoseconds;
};
int ed_budget(uint32_t application, unsigned megabits, struct ed_budget *out);
int ed_capture(const uint32_t *times, size_t count, int32_t offset,
               unsigned offset_known, uint32_t uncertainty, uint64_t *lower,
               uint64_t *upper);
int ed_first_failure(unsigned passed_mask, unsigned *index);
#endif
