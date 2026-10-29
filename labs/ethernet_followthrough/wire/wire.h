#ifndef EW_WIRE_H
#define EW_WIRE_H
#include "ethernet_depth.h"
enum {
  EW_OK,
  EW_MORE,
  EW_READY,
  EW_FULL,
  EW_INVALID,
  EW_POISON,
  EW_EOF,
  EW_TRUNCATED,
  EW_STUB = 99
};
struct ew_stream {
  unsigned char data[256];
  unsigned length, used, prefix, phase;
};
int ew_feed(struct ew_stream *, const unsigned char *, size_t, size_t *);
int ew_take(struct ew_stream *, unsigned char *, size_t, size_t *);
int ew_end(struct ew_stream *);
struct ew_vlan {
  struct ed_frame frame;
  unsigned priority, vid;
};
int ew_vlan_build(const unsigned char *, const unsigned char *, unsigned,
                  unsigned, unsigned, const unsigned char *, size_t,
                  unsigned char *, size_t, size_t *);
int ew_vlan_parse(const unsigned char *, size_t, struct ew_vlan *);
int ew_vlan_pop(const unsigned char *, size_t, unsigned char *, size_t,
                size_t *);
int ew_bridge(struct ed_bridge *, const unsigned char *, const unsigned char *,
              unsigned, uint32_t, unsigned, unsigned *);
#endif
