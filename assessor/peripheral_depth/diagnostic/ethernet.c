#include "ethernet.h"
#ifndef DG_MUTANT
#define DG_MUTANT 0
#endif
static const uint8_t destination[6] = {2, 0, 0, 0, 0, 2};
static const uint8_t source[6] = {2, 0, 0, 0, 0, 1};
int dg_ethernet_build(const struct dg_message *m, uint8_t *out, size_t cap,
                      size_t *written) {
  uint8_t wire[36];
  size_t n;
  int rc = dg_encode(m, wire, sizeof wire, &n);
  if (rc)
    return rc;
  rc = ed_frame_build(destination, source, (uint16_t)n, wire, n, 0, out, cap,
                      written);
  return rc == ED_OK ? DG_OK : rc == ED_FULL ? DG_FULL : DG_INVALID;
}
int dg_ethernet_accept(struct dg_session *s, const uint8_t *p, size_t n,
                       struct dg_message *out) {
  struct ed_frame f;
  int rc = ed_frame_parse(p, n, 0, &f);
  if (rc != ED_OK)
    return DG_FRAME;
  for (unsigned i = 0; i < 6; ++i)
    if (f.destination[i] != destination[i] || f.source[i] != source[i])
      return DG_FRAME;
  if (f.field > 36 || f.field < 4)
    return DG_FRAME;
  return dg_accept(s, f.payload,
                   DG_MUTANT == 4 ? f.stored_payload : f.payload_length, out);
}
int dg_descriptors(struct dg_session *s, struct ed_rx *rx, const struct ed_descriptor *d, size_t n,
                   struct dg_message *out) {
  if (!s || !rx || !out)
    return DG_INVALID;
  struct ed_borrow b;
  int rc = ed_rx_acquire(rx, d, n, &b);
  if (rc != ED_OK)
    return rc == ED_BUSY ? DG_BUSY : DG_FRAME;
  uint8_t frame[60];
  size_t length = (size_t)b.first_length + b.second_length;
  rc = DG_FRAME;
  /* This route deliberately permits only minimum-size diagnostic frames. */
  if (length == sizeof frame) {
    for (size_t i = 0; i < b.first_length; ++i)
      frame[i] = b.first[i];
    for (size_t i = 0; i < b.second_length; ++i)
      frame[b.first_length + i] = b.second[i];
    rc = dg_ethernet_accept(s, frame, length, out);
  }
  if (DG_MUTANT != 5 && ed_rx_release(rx, b.token) != ED_OK)
    return DG_IO;
  return rc;
}
int dg_publish(struct ed_tx *tx, unsigned slot, uint32_t token, const struct dg_message *m,
               void (*barrier)(void *), void *context) {
  if (!tx || !barrier)
    return DG_INVALID;
  uint8_t frame[60];
  size_t n;
  int rc = dg_ethernet_build(m, frame, sizeof frame, &n);
  if (rc)
    return rc;
  rc = ed_tx_prepare(tx, slot, token, frame, n);
  if (rc != ED_OK)
    return rc == ED_STALE ? DG_STALE : DG_BUSY;
  if (ed_tx_tail(tx, slot, token) != ED_OK)
    return DG_IO;
  rc = ed_tx_publish(tx, slot, token, barrier, context);
  /* BUSY retains the prepared tail-owned slot. Retry publication, not prepare.
   */
  return rc == ED_OK ? DG_OK : rc == ED_BUSY ? DG_BUSY : DG_IO;
}
