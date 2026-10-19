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
typedef struct {
  void *context;
  int (*submit)(void *, const uint8_t *, unsigned, uint32_t, uint32_t,
                uint32_t);
  int (*poll)(void *, uint32_t, uint32_t);
  int (*cancel)(void *, uint32_t);
} PlTransport;
typedef struct {
  uint8_t bytes[8];
  uint32_t token, active, result;
} PlService;
typedef struct {
  uint32_t token, status, may_have_committed;
} PlResult;
int pl_encode(uint8_t, uint8_t, uint8_t[8]);
int pl_send(PlService *, PlTransport, uint8_t, uint8_t, uint32_t, uint32_t,
            uint32_t);
int pl_poll(PlService *, PlTransport, uint32_t, uint32_t, PlResult *);
int pl_cancel(PlService *, PlTransport, uint32_t);
typedef struct {
  uint8_t raw[3];
  unsigned active, nibbles;
} PlParser;
typedef struct {
  uint8_t sequence, value;
} PlCommand;
int pl_feed(PlParser *, uint8_t, PlCommand *);
#endif
