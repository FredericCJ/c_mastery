#include "tcp.h"
/* Deliberately incomplete learner implementation. */
int nc_rtt_init(NcRtt *a0, unsigned a1, unsigned a2) {
  (void)a0;
  (void)a1;
  (void)a2;
  return NC_STUB;
}
int nc_rtt_sample(NcRtt *a0, unsigned a1, unsigned a2) {
  (void)a0;
  (void)a1;
  (void)a2;
  return NC_STUB;
}
int nc_init(NcStack *a0, NiConfig a1, uint32_t a2, uint32_t a3, unsigned a4,
            unsigned a5) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  (void)a4;
  (void)a5;
  return NC_STUB;
}
int nc_receive(NcStack *a0, const unsigned char *a1, size_t a2, uint64_t a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  return NC_STUB;
}
int nc_tick(NcStack *a0, uint64_t a1) {
  (void)a0;
  (void)a1;
  return NC_STUB;
}
int nc_offer(NcStack *a0, const void *a1, size_t a2) {
  (void)a0;
  (void)a1;
  (void)a2;
  return NC_STUB;
}
int nc_close(NcStack *a0) {
  (void)a0;
  return NC_STUB;
}
int nc_abort(NcStack *a0, uint64_t a1) {
  (void)a0;
  (void)a1;
  return NC_STUB;
}
int nc_prepare(NcStack *a0, NcPacket *a1) {
  (void)a0;
  (void)a1;
  return NC_STUB;
}
int nc_submit(NcStack *a0, NcTicket a1, unsigned a2, uint64_t a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  return NC_STUB;
}
int nc_dma_done(NcStack *a0, NcTicket a1) {
  (void)a0;
  (void)a1;
  return NC_STUB;
}
int nc_consume(NcStack *a0, size_t a1) {
  (void)a0;
  (void)a1;
  return NC_STUB;
}
int nc_http_pump(NcStack *a0, HttpParser *a1, size_t a2, size_t *a3) {
  (void)a0;
  (void)a1;
  (void)a2;
  (void)a3;
  return NC_STUB;
}
int nc_http_eof(NcStack *a0, HttpParser *a1) {
  (void)a0;
  (void)a1;
  return NC_STUB;
}
unsigned nc_usable(const NcStack *a0) {
  (void)a0;
  return NC_STUB;
}
