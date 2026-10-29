#ifndef CMASTERY_ADF_H
#define CMASTERY_ADF_H
#include "adt.h"
#include <stdint.h>

enum adf_status
{
    ADF_OK,
    ADF_INVALID,
    ADF_UNSUPPORTED
};

struct adf_capabilities
{
    unsigned version;
    uint32_t bits;
};

struct adf_offer
{
    uint32_t required, advisory;
};

struct adf_agreement
{
    uint32_t common, required, advisory, enabled;
};
/* All inputs are values. Refusal leaves a nonnull live output unchanged.
   Capabilities are operational claims, not authentication or authorization. */
enum adf_status adf_capabilities(unsigned version, uint32_t bits,
                                 struct adf_capabilities *out);
enum adf_status adf_union(struct ad_flags a, struct ad_flags b,
                          struct ad_flags *out);
enum adf_status adf_intersection(struct ad_flags a, struct ad_flags b,
                                 struct ad_flags *out);
enum adf_status adf_negotiate(struct adf_capabilities local,
                              struct adf_capabilities peer,
                              struct adf_offer local_offer,
                              struct adf_offer peer_offer,
                              struct adf_agreement *out);
#endif
