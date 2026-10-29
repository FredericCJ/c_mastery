#include "unicode_stream.h"
void adu_init(struct adu_stream *s) { (void)s; }
enum adu_status adu_feed(struct adu_stream *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return ADU_INVALID;
}
enum adu_status adu_finish(struct adu_stream *s, struct adu_summary *o) {
  (void)s;
  (void)o;
  return ADU_INVALID;
}
enum adu_status adu_cancel(struct adu_stream *s) {
  (void)s;
  return ADU_INVALID;
}
void adu_owner_init(struct adu_owner *s) { (void)s; }
enum adu_status adu_owner_set(struct adu_owner *s, const unsigned char *p, size_t n,
                        struct ado_ops o) {
  (void)s;
  (void)p;
  (void)n;
  (void)o;
  return ADU_INVALID;
}
enum adu_status adu_owner_view(const struct adu_owner *s, struct ad_text *o) {
  (void)s;
  (void)o;
  return ADU_INVALID;
}
void adu_owner_drop(struct adu_owner *s) { (void)s; }
