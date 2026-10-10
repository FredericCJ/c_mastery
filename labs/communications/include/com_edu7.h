#ifndef COM_EDU7_H
#define COM_EDU7_H
#include <stddef.h>
#include <stdint.h>
typedef struct {
  uint8_t kind, source, destination, length, payload[40];
} EduFrame;
typedef struct {
  uint8_t bytes[47];
  size_t length;
  unsigned escape, discard, active;
} EduFramer;
uint16_t edu_crc(const void *, size_t);
int edu_frame_encode(const EduFrame *, void *, size_t, size_t *);
/* 1 complete valid frame, 0 incomplete, -1 drop; UART error invalidates frame.
 */
int edu_frame_byte(EduFramer *, uint8_t, int uart_error, EduFrame *);
typedef struct {
  uint8_t source, destination, hop, length, payload[34];
} EduDatagram;
int edu_network_encode(const EduDatagram *, void *, size_t *);
int edu_network_decode(const void *, size_t, EduDatagram *);
/* Router consumes no presentation/session fields; output owns its payload. */
int edu_route(const EduDatagram *, unsigned ingress, EduDatagram *,
              unsigned *egress, uint8_t *next_link);
typedef struct {
  uint8_t type, source_port, destination_port, index, count, credit;
  uint16_t id;
  uint8_t data[24];
  size_t length;
} EduFragment;
int edu_fragment_encode(const EduFragment *, void *, size_t *);
int edu_fragment_decode(const void *, size_t, EduFragment *);
typedef struct {
  uint8_t bytes[120];
  size_t length;
  uint16_t id;
  uint8_t source, destination, index, count, credit;
  unsigned active, retries, waiting, blocked;
  uint64_t due, blocked_since;
} EduTx;
typedef struct {
  uint8_t bytes[120], last[24];
  size_t length, last_length;
  uint16_t id, completed;
  uint8_t source, destination, index, count, last_count;
  unsigned active, ready, publications;
} EduRx;
int edu_tx_begin(EduTx *, uint16_t, uint8_t, uint8_t, const void *, size_t,
                 uint64_t);
/* 1 emits DATA/PROBE,0 waits,-1 failure; copies immutable bytes each attempt.
 */
int edu_tx_poll(EduTx *, uint64_t, EduFragment *);
int edu_tx_ack(EduTx *, const EduFragment *, uint64_t);
/* ACK output on accepted/duplicate DATA; publication count changes once. */
int edu_rx_accept(EduRx *, const EduFragment *, EduFragment *ack);
void edu_rx_release(EduRx *);
typedef struct {
  uint8_t verb;
  uint16_t request;
  int16_t value;
  unsigned has_value, has_unit;
  uint8_t label[16], label_length;
} EduRecord;
int edu_record_decode(const void *, size_t, EduRecord *);
int edu_record_encode(const EduRecord *, void *, size_t *);
int edu_application(const EduRecord *, int16_t *setpoint, EduRecord *reply);
typedef struct {
  uint16_t session, epoch, checkpoint;
  uint8_t holder, last[80];
  size_t last_length;
  unsigned open, token_pending, effects;
} EduSession;
/* Original L5 bytes: type,sidBE16,epochBE16,checkpointBE16,payload.
   Return 1 new DATA,2 duplicate,0 control,-1 reject. No durable state. */
int edu_session_receive(EduSession *, uint8_t sender, const void *, size_t,
                        const uint8_t **data, size_t *length);
int edu_session_token_begin(EduSession *, uint8_t sender);
void edu_session_token_complete(EduSession *, uint8_t new_holder, int success);
typedef struct {
  unsigned driving, phase;
  uint64_t due;
} EduPhysical;
int edu_phy_begin(EduPhysical *, uint64_t now_us);
int edu_phy_started(EduPhysical *, uint64_t now_us);
int edu_phy_complete(EduPhysical *, uint64_t now_us, int uart_tc);
int edu_phy_ready(EduPhysical *, uint64_t now_us);
#endif
