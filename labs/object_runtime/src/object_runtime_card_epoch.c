#include "object_runtime_card_epoch.h"
enum or_status orcc_init(struct orc_cards *c, unsigned w, int p) {
  (void)c;
  (void)w;
  (void)p;
  return OR_STATE;
}
enum or_status orcc_store(struct orc_cards *c, unsigned o, unsigned f, unsigned t) {
  (void)c;
  (void)o;
  (void)f;
  (void)t;
  return OR_STATE;
}
enum or_status orcc_prepare(struct orc_cards *c, unsigned n, struct orcc_snapshot *s) {
  (void)c;
  (void)n;
  (void)s;
  return OR_STATE;
}
enum or_status orcc_scan_commit(struct orc_cards *c, const struct orcc_snapshot *s, uint16_t *r,
                          unsigned *n) {
  (void)c;
  (void)s;
  (void)r;
  (void)n;
  return OR_STATE;
}
enum or_status orcc_close(struct orc_cards *c) {
  (void)c;
  return OR_STATE;
}
