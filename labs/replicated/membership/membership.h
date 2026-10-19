#ifndef CMASTERY_REPLICATED_MEMBERSHIP_H
#define CMASTERY_REPLICATED_MEMBERSHIP_H
#include "replicated.h"
#include <stddef.h>
#include <stdint.h>
enum {
  RPM_OK,
  RPM_INVALID,
  RPM_FULL,
  RPM_RANGE,
  RPM_IDENTITY,
  RPM_AUTH,
  RPM_POLICY,
  RPM_CORRUPT,
  RPM_UNIMPLEMENTED
};
typedef struct {
  uint64_t uuid, self, ids[6];
  RPPN blocks[2];
  unsigned count;
} RpmCounter;
typedef struct {
  uint64_t uuid, writer;
  unsigned char key[32];
  uint32_t positive_limit, negative_limit;
  unsigned active;
} RpmPermit;
int rpm_make(uint64_t uuid, uint64_t self, RpmCounter *out);
int rpm_add(const RpmCounter *local, int negative, uint32_t amount,
            RpmCounter *out);
int rpm_replace(const RpmCounter *local, uint64_t fresh, RpmCounter *out);
int rpm_merge(const RpmCounter *local, const RpmCounter *admitted,
              RpmCounter *out);
int rpm_value(const RpmCounter *state, int64_t *out);
int rpm_encode(const RpmCounter *state, unsigned char out[160]);
int rpm_decode(const unsigned char wire[160], RpmCounter *out);
int rpm_seal(const RpmCounter *writer, const unsigned char key[32],
             unsigned char out[64]);
int rpm_admit(const RpmCounter *local, const RpmPermit *permit,
              const unsigned char wire[64], RpmCounter *out);
#endif
