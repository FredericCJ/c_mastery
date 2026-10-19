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
typedef struct {
  unsigned char data[256];
  unsigned length, used, prefix, phase;
} EwStream;
int ew_feed(EwStream *, const unsigned char *, size_t, size_t *);
int ew_take(EwStream *, unsigned char *, size_t, size_t *);
int ew_end(EwStream *);
typedef struct {
  EDFrame frame;
  unsigned priority, vid;
} EwVlan;
int ew_vlan_build(const unsigned char *, const unsigned char *, unsigned,
                  unsigned, unsigned, const unsigned char *, size_t,
                  unsigned char *, size_t, size_t *);
int ew_vlan_parse(const unsigned char *, size_t, EwVlan *);
int ew_vlan_pop(const unsigned char *, size_t, unsigned char *, size_t,
                size_t *);
int ew_bridge(EDBridge *, const unsigned char *, const unsigned char *,
              unsigned, uint32_t, unsigned, unsigned *);
#endif
