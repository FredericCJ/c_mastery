#include "flags_negotiation.h"
#ifndef ADF_MUTATION
#define ADF_MUTATION 0
#endif
AdfStatus adf_capabilities(unsigned version, uint32_t bits,
                           AdfCapabilities *out) {
  if (!out)
    return ADF_INVALID;
  if (version == 1) {
    ADFlags admitted;
    if (!ad_flags(bits, &admitted))
      return ADF_INVALID;
  } else if (version == 2) {
    if ((bits & ~UINT32_C(63)) != 0)
      return ADF_INVALID;
  } else
    return ADF_INVALID;
  *out = (AdfCapabilities){version, bits};
  return ADF_OK;
}
AdfStatus adf_union(ADFlags a, ADFlags b, ADFlags *out) {
  ADFlags admitted;
  if (!out || !ad_flags(a.bits, &admitted) || !ad_flags(b.bits, &admitted))
    return ADF_INVALID;
  out->bits = ADF_MUTATION == 3 ? (a.bits & b.bits) : (a.bits | b.bits);
  return ADF_OK;
}
AdfStatus adf_intersection(ADFlags a, ADFlags b, ADFlags *out) {
  ADFlags admitted;
  if (!out || !ad_flags(a.bits, &admitted) || !ad_flags(b.bits, &admitted))
    return ADF_INVALID;
  out->bits = ADF_MUTATION == 4 ? (a.bits | b.bits) : (a.bits & b.bits);
  return ADF_OK;
}
AdfStatus adf_negotiate(AdfCapabilities local, AdfCapabilities peer,
                        AdfOffer local_offer, AdfOffer peer_offer,
                        AdfAgreement *out) {
  AdfCapabilities admitted;
  if (!out ||
      adf_capabilities(local.version, local.bits, &admitted) != ADF_OK ||
      adf_capabilities(peer.version, peer.bits, &admitted) != ADF_OK)
    return ADF_INVALID;
  AdfAgreement next;
  next.common =
      ADF_MUTATION == 1 ? (local.bits | peer.bits) : (local.bits & peer.bits);
  next.required = local_offer.required | peer_offer.required;
  if ((next.required & ~next.common) != 0)
    return ADF_UNSUPPORTED;
  next.advisory = local_offer.advisory | peer_offer.advisory;
  if (ADF_MUTATION == 2)
    next.advisory &= next.common;
  next.enabled = (next.required | next.advisory) & next.common;
  *out = next;
  return ADF_OK;
}
