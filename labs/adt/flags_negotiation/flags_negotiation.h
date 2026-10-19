#ifndef CMASTERY_ADF_H
#define CMASTERY_ADF_H
#include "adt.h"
#include <stdint.h>
typedef enum { ADF_OK, ADF_INVALID, ADF_UNSUPPORTED } AdfStatus;
typedef struct {
  unsigned version;
  uint32_t bits;
} AdfCapabilities;
typedef struct {
  uint32_t required, advisory;
} AdfOffer;
typedef struct {
  uint32_t common, required, advisory, enabled;
} AdfAgreement;
/* All inputs are values. Refusal leaves a nonnull live output unchanged.
   Capabilities are operational claims, not authentication or authorization. */
AdfStatus adf_capabilities(unsigned version, uint32_t bits,
                           AdfCapabilities *out);
AdfStatus adf_union(ADFlags a, ADFlags b, ADFlags *out);
AdfStatus adf_intersection(ADFlags a, ADFlags b, ADFlags *out);
AdfStatus adf_negotiate(AdfCapabilities local, AdfCapabilities peer,
                        AdfOffer local_offer, AdfOffer peer_offer,
                        AdfAgreement *out);
#endif
