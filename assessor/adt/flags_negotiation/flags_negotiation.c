#include "flags_negotiation.h"
#ifndef ADF_MUTATION
#define ADF_MUTATION 0
#endif
enum adf_status adf_capabilities(unsigned version, uint32_t bits,
                           struct adf_capabilities *out) {
  if (!out)
    return ADF_INVALID;
  if (version == 1) {
    struct ad_flags admitted;
    if (!ad_flags(bits, &admitted))
      return ADF_INVALID;
  } else if (version == 2) {
    if ((bits & ~UINT32_C(63)) != 0)
      return ADF_INVALID;
  } else
    return ADF_INVALID;
  *out = (struct adf_capabilities){version, bits};
  return ADF_OK;
}
enum adf_status adf_union(struct ad_flags a, struct ad_flags b, struct ad_flags *out) {
  struct ad_flags admitted;
  if (!out || !ad_flags(a.bits, &admitted) || !ad_flags(b.bits, &admitted))
    return ADF_INVALID;
  out->bits = ADF_MUTATION == 3 ? (a.bits & b.bits) : (a.bits | b.bits);
  return ADF_OK;
}
enum adf_status adf_intersection(struct ad_flags a, struct ad_flags b, struct ad_flags *out) {
  struct ad_flags admitted;
  if (!out || !ad_flags(a.bits, &admitted) || !ad_flags(b.bits, &admitted))
    return ADF_INVALID;
  out->bits = ADF_MUTATION == 4 ? (a.bits | b.bits) : (a.bits & b.bits);
  return ADF_OK;
}
enum adf_status adf_negotiate(struct adf_capabilities local, struct adf_capabilities peer,
                        struct adf_offer local_offer, struct adf_offer peer_offer,
                        struct adf_agreement *out) {
  struct adf_capabilities admitted;
  if (!out ||
      adf_capabilities(local.version, local.bits, &admitted) != ADF_OK ||
      adf_capabilities(peer.version, peer.bits, &admitted) != ADF_OK)
    return ADF_INVALID;
  struct adf_agreement next;
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
