#include "com_edu7.h"
/* Buildable exercises: every operation still needs implementation. */
uint16_t edu_crc(const void *v, size_t n) { return 0; }
int edu_frame_encode(const EduFrame *f, void *v, size_t cap, size_t *length) {
  return -1;
}
int edu_frame_byte(EduFramer *p, uint8_t c, int fault, EduFrame *out) {
  return -1;
}
int edu_network_encode(const EduDatagram *d, void *v, size_t *n) { return -1; }
int edu_network_decode(const void *v, size_t n, EduDatagram *d) { return -1; }
int edu_route(const EduDatagram *in, unsigned ingress, EduDatagram *out,
              unsigned *egress, uint8_t *next) {
  return -1;
}
int edu_fragment_encode(const EduFragment *f, void *v, size_t *n) { return -1; }
int edu_fragment_decode(const void *v, size_t n, EduFragment *f) { return -1; }
int edu_tx_begin(EduTx *t, uint16_t id, uint8_t src, uint8_t dst, const void *v,
                 size_t n, uint64_t now) {
  return -1;
}
int edu_tx_poll(EduTx *t, uint64_t now, EduFragment *out) { return -1; }
int edu_tx_ack(EduTx *t, const EduFragment *a, uint64_t now) { return -1; }
int edu_rx_accept(EduRx *r, const EduFragment *f, EduFragment *ack) {
  return -1;
}
void edu_rx_release(EduRx *r) { return; }
int edu_record_decode(const void *v, size_t n, EduRecord *out) { return -1; }
int edu_record_encode(const EduRecord *r, void *v, size_t *n) { return -1; }
int edu_application(const EduRecord *r, int16_t *set, EduRecord *reply) {
  return -1;
}
int edu_session_receive(EduSession *s, uint8_t sender, const void *v, size_t n,
                        const uint8_t **data, size_t *len) {
  return -1;
}
int edu_session_token_begin(EduSession *s, uint8_t sender) { return -1; }
void edu_session_token_complete(EduSession *s, uint8_t holder, int ok) {
  return;
}
int edu_phy_begin(EduPhysical *p, uint64_t now) { return -1; }
int edu_phy_started(EduPhysical *p, uint64_t now) { return -1; }
int edu_phy_complete(EduPhysical *p, uint64_t now, int tc) { return -1; }
int edu_phy_ready(EduPhysical *p, uint64_t now) { return -1; }
