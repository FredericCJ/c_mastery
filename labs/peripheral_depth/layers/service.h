#ifndef PERIPHERAL_LAYERS_SERVICE_H
#define PERIPHERAL_LAYERS_SERVICE_H
#include <stdint.h>
enum {
  PL_OK,
  PL_INVALID,
  PL_BUSY,
  PL_STALE,
  PL_CANCELLED,
  PL_IO,
  PL_TIMEOUT,
  PL_FRAME,
  PL_UNIMPLEMENTED
};
struct pl_transport {
  void *context;
  int (*submit)(void *, const uint8_t *, unsigned, uint32_t, uint32_t,
                uint32_t);
  int (*poll)(void *, uint32_t, uint32_t);
  int (*cancel)(void *, uint32_t);
};
struct pl_service {
  uint8_t bytes[8];
  uint32_t token, active, result;
};
struct pl_result {
  uint32_t token, status, may_have_committed;
};
int pl_encode(uint8_t, uint8_t, uint8_t[8]);
int pl_send(struct pl_service *, struct pl_transport, uint8_t, uint8_t, uint32_t, uint32_t,
            uint32_t);
int pl_poll(struct pl_service *, struct pl_transport, uint32_t, uint32_t, struct pl_result *);
int pl_cancel(struct pl_service *, struct pl_transport, uint32_t);
struct pl_parser {
  uint8_t raw[3];
  unsigned active, nibbles;
};
struct pl_command {
  uint8_t sequence, value;
};
int pl_feed(struct pl_parser *, uint8_t, struct pl_command *);
#endif
