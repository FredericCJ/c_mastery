#include "transport.h"
int rpo_wire_read(RpoReader r, void *c, unsigned n, size_t l,
                  unsigned char *o) {
  (void)r;
  (void)c;
  (void)n;
  (void)l;
  (void)o;
  return RPL_UNIMPLEMENTED;
}
int rpo_wire_write(RpoWriter w, void *c, unsigned n, const unsigned char *b,
                   size_t l) {
  (void)w;
  (void)c;
  (void)n;
  (void)b;
  (void)l;
  return RPL_UNIMPLEMENTED;
}
int rpo_reply_encode(RpoReply r, unsigned char o[24]) {
  (void)r;
  (void)o;
  return RPL_UNIMPLEMENTED;
}
int rpo_reply_decode(const unsigned char i[24], RpoReply *o) {
  (void)i;
  (void)o;
  return RPL_UNIMPLEMENTED;
}
