#include "flags_negotiation.h"
enum adf_status adf_capabilities(unsigned v, uint32_t b, struct adf_capabilities *out) {
  (void)v;
  (void)b;
  (void)out;
  return ADF_INVALID;
}
enum adf_status adf_union(struct ad_flags a, struct ad_flags b, struct ad_flags *out) {
  (void)a;
  (void)b;
  (void)out;
  return ADF_INVALID;
}
enum adf_status adf_intersection(struct ad_flags a, struct ad_flags b, struct ad_flags *out) {
  (void)a;
  (void)b;
  (void)out;
  return ADF_INVALID;
}
enum adf_status adf_negotiate(struct adf_capabilities a, struct adf_capabilities b, struct adf_offer c,
                        struct adf_offer d, struct adf_agreement *out) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)out;
  return ADF_INVALID;
}
