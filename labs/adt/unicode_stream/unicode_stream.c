#include "unicode_stream.h"
void adu_init(AduStream *s) { (void)s; }
AduStatus adu_feed(AduStream *s, const unsigned char *p, size_t n) {
  (void)s;
  (void)p;
  (void)n;
  return ADU_INVALID;
}
AduStatus adu_finish(AduStream *s, AduSummary *o) {
  (void)s;
  (void)o;
  return ADU_INVALID;
}
AduStatus adu_cancel(AduStream *s) {
  (void)s;
  return ADU_INVALID;
}
void adu_owner_init(AduOwner *s) { (void)s; }
AduStatus adu_owner_set(AduOwner *s, const unsigned char *p, size_t n,
                        AdoOps o) {
  (void)s;
  (void)p;
  (void)n;
  (void)o;
  return ADU_INVALID;
}
AduStatus adu_owner_view(const AduOwner *s, ADText *o) {
  (void)s;
  (void)o;
  return ADU_INVALID;
}
void adu_owner_drop(AduOwner *s) { (void)s; }
