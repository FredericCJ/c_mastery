#include "membership.h"
int rpm_make(uint64_t uuid, uint64_t self, RpmCounter *out) {
  (void)uuid;
  (void)self;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_add(const RpmCounter *local, int negative, uint32_t amount,
            RpmCounter *out) {
  (void)local;
  (void)negative;
  (void)amount;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_replace(const RpmCounter *local, uint64_t fresh, RpmCounter *out) {
  (void)local;
  (void)fresh;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_merge(const RpmCounter *local, const RpmCounter *admitted,
              RpmCounter *out) {
  (void)local;
  (void)admitted;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_value(const RpmCounter *state, int64_t *out) {
  (void)state;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_encode(const RpmCounter *state, unsigned char out[160]) {
  (void)state;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_decode(const unsigned char wire[160], RpmCounter *out) {
  (void)wire;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_seal(const RpmCounter *writer, const unsigned char key[32],
             unsigned char out[64]) {
  (void)writer;
  (void)key;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
int rpm_admit(const RpmCounter *local, const RpmPermit *permit,
              const unsigned char wire[64], RpmCounter *out) {
  (void)local;
  (void)permit;
  (void)wire;
  (void)out;
  return RPM_UNIMPLEMENTED;
}
