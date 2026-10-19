#include "flags_negotiation.h"
AdfStatus adf_capabilities(unsigned v, uint32_t b, AdfCapabilities *out) {
  (void)v;
  (void)b;
  (void)out;
  return ADF_INVALID;
}
AdfStatus adf_union(ADFlags a, ADFlags b, ADFlags *out) {
  (void)a;
  (void)b;
  (void)out;
  return ADF_INVALID;
}
AdfStatus adf_intersection(ADFlags a, ADFlags b, ADFlags *out) {
  (void)a;
  (void)b;
  (void)out;
  return ADF_INVALID;
}
AdfStatus adf_negotiate(AdfCapabilities a, AdfCapabilities b, AdfOffer c,
                        AdfOffer d, AdfAgreement *out) {
  (void)a;
  (void)b;
  (void)c;
  (void)d;
  (void)out;
  return ADF_INVALID;
}
