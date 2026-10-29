#include "hashing.h"
#include "membership.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static struct rpm_permit permit(uint64_t id) {
  struct rpm_permit p = {0};
  unsigned i;
  p.uuid = 9;
  p.writer = id;
  p.active = 1;
  p.positive_limit = p.negative_limit = 100;
  for (i = 0; i < 32; ++i)
    p.key[i] = (unsigned char)(i + id);
  return p;
}
static int equal(const struct rpm_counter *a, const struct rpm_counter *b) {
  unsigned char x[160], y[160];
  return rpm_encode(a, x) == RPM_OK && rpm_encode(b, y) == RPM_OK &&
         !memcmp(x, y, 160);
}
static int join(void) {
  struct rpm_counter a, b, c, d, saved;
  int64_t value;
  unsigned i;
  CHECK(rpm_make(9, 10, &a) == RPM_OK);
  CHECK(rpm_add(&a, 0, 5, &b) == RPM_OK);
  a = b;
  CHECK(rpm_make(9, 20, &b) == RPM_OK);
  CHECK(rpm_add(&b, 1, 3, &c) == RPM_OK);
  b = c;
  CHECK(rpm_merge(&a, &b, &c) == RPM_OK);
  CHECK(rpm_value(&c, &value) == RPM_OK && value == 2);
  CHECK(rpm_merge(&c, &b, &d) == RPM_OK && equal(&c, &d));
  CHECK(rpm_replace(&c, 30, &d) == RPM_OK);
  CHECK(d.count == 3 && d.self == 30);
  CHECK(rpm_value(&d, &value) == RPM_OK && value == 2);
  CHECK(rpm_add(&d, 0, 4, &c) == RPM_OK);
  CHECK(rpm_merge(&c, &a, &d) == RPM_OK);
  CHECK(rpm_value(&d, &value) == RPM_OK && value == 6);
  for (i = 40; i <= 60; i += 10) {
    CHECK(rpm_replace(&d, i, &c) == RPM_OK);
    d = c;
  }
  saved = d;
  CHECK(rpm_replace(&d, 70, &c) == RPM_FULL);
  CHECK(equal(&d, &saved));
  CHECK(rpm_replace(&d, 10, &c) == RPM_IDENTITY);
  CHECK(rpm_add(&a, 0, UINT32_MAX, &c) == RPM_RANGE);
  return 0;
}
static int laws(void) {
  struct rpm_counter states[16], a, b, c, d;
  unsigned i, j, k;
  for (i = 0; i < 16; ++i) {
    CHECK(rpm_make(9, 10, &states[i]) == RPM_OK);
    states[i].count = 2;
    states[i].ids[1] = 20;
    states[i].blocks[0].positive[0] = i & 1;
    states[i].blocks[0].negative[0] = (i >> 1) & 1;
    states[i].blocks[0].positive[1] = (i >> 2) & 1;
    states[i].blocks[0].negative[1] = (i >> 3) & 1;
  }
  for (i = 0; i < 16; ++i) {
    CHECK(rpm_merge(&states[i], &states[i], &a) == RPM_OK &&
          equal(&a, &states[i]));
    for (j = 0; j < 16; ++j) {
      CHECK(rpm_merge(&states[i], &states[j], &a) == RPM_OK);
      CHECK(rpm_merge(&states[j], &states[i], &b) == RPM_OK && equal(&a, &b));
      for (k = 0; k < 16; ++k) {
        CHECK(rpm_merge(&a, &states[k], &c) == RPM_OK);
        CHECK(rpm_merge(&states[j], &states[k], &b) == RPM_OK);
        CHECK(rpm_merge(&states[i], &b, &d) == RPM_OK && equal(&c, &d));
      }
    }
  }
  puts("MEMBERSHIP_LAWS 4368");
  return 0;
}
static int wire(void) {
  struct rpm_counter a, b, saved;
  unsigned char bytes[160], changed[160];
  unsigned i;
  CHECK(rpm_make(9, 10, &a) == RPM_OK);
  CHECK(rpm_add(&a, 0, 7, &b) == RPM_OK);
  a = b;
  CHECK(rpm_encode(&a, bytes) == RPM_OK);
  CHECK(rpm_decode(bytes, &b) == RPM_OK && equal(&a, &b));
  saved = b;
  for (i = 0; i < 1280; ++i) {
    memcpy(changed, bytes, 160);
    changed[i / 8] ^= (unsigned char)(1u << (i % 8));
    CHECK(rpm_decode(changed, &b) == RPM_CORRUPT);
    CHECK(equal(&saved, &b));
  }
  memcpy(changed, bytes, 160);
  changed[24] = 1;
  (void)rph_hash(changed, 128, changed + 128);
  CHECK(rpm_decode(changed, &b) == RPM_CORRUPT);
  puts("MEMBERSHIP_BITS 1280");
  return 0;
}
static int auth(void) {
  struct rpm_counter a, b, out, saved;
  struct rpm_permit p = permit(20), unlimited;
  unsigned char bytes[64], changed[64];
  unsigned i;
  int64_t value;
  CHECK(rpm_make(9, 10, &a) == RPM_OK);
  CHECK(rpm_make(9, 20, &b) == RPM_OK);
  CHECK(rpm_add(&b, 0, 9, &out) == RPM_OK);
  b = out;
  CHECK(rpm_seal(&b, p.key, bytes) == RPM_OK);
  CHECK(rpm_admit(&a, &p, bytes, &out) == RPM_OK);
  CHECK(rpm_value(&out, &value) == RPM_OK && value == 9);
  saved = out;
  for (i = 32; i < 64; ++i) {
    memcpy(changed, bytes, 64);
    changed[i] ^= 1;
    CHECK(rpm_admit(&a, &p, changed, &out) == RPM_AUTH);
    CHECK(equal(&saved, &out));
  }
  /* Key holder20 may sign bytes naming30, but its permit owns only20. */
  memcpy(changed, bytes, 64);
  changed[23] = 30;
  (void)rph_hmac(p.key, 32, changed, 32, changed + 32);
  CHECK(rpm_admit(&a, &p, changed, &out) == RPM_IDENTITY);
  CHECK(equal(&saved, &out));
  CHECK(rpm_add(&b, 0, UINT32_MAX - 9, &out) == RPM_OK);
  b = out;
  CHECK(rpm_seal(&b, p.key, bytes) == RPM_OK);
  out = saved;
  CHECK(rpm_admit(&a, &p, bytes, &out) == RPM_POLICY);
  CHECK(equal(&saved, &out));
  /* A valid tag cannot establish a business entitlement. Unrestricted policy
     admits the poisoned maximum and the writer can no longer increase it. */
  unlimited = p;
  unlimited.positive_limit = UINT32_MAX;
  CHECK(rpm_admit(&a, &unlimited, bytes, &out) == RPM_OK);
  CHECK(rpm_value(&out, &value) == RPM_OK && value == UINT32_MAX);
  CHECK(rpm_add(&b, 0, 1, &out) == RPM_RANGE);
  puts("MEMBERSHIP_AUTH forged-other=refused valid-high=policy-dependent");
  return 0;
}
static int replay(void) {
  struct rpm_counter a, b, next, out;
  struct rpm_permit p = permit(20);
  unsigned char old[64], newer[64];
  int64_t value;
  CHECK(rpm_make(9, 10, &a) == RPM_OK);
  CHECK(rpm_make(9, 20, &b) == RPM_OK);
  CHECK(rpm_add(&b, 0, 2, &next) == RPM_OK);
  b = next;
  CHECK(rpm_seal(&b, p.key, old) == RPM_OK);
  CHECK(rpm_add(&b, 0, 3, &next) == RPM_OK);
  b = next;
  CHECK(rpm_seal(&b, p.key, newer) == RPM_OK);
  CHECK(rpm_admit(&a, &p, newer, &out) == RPM_OK);
  a = out;
  CHECK(rpm_admit(&a, &p, old, &out) == RPM_OK);
  CHECK(rpm_value(&out, &value) == RPM_OK && value == 5);
  p.active = 0;
  a = out;
  CHECK(rpm_admit(&a, &p, newer, &out) == RPM_POLICY);
  CHECK(equal(&a, &out));
  p.active = 1;
  p.uuid = 10;
  CHECK(rpm_admit(&a, &p, newer, &out) == RPM_IDENTITY);
  CHECK(equal(&a, &out));
  return 0;
}
static void printhex(const unsigned char *p, size_t n) {
  size_t i;
  for (i = 0; i < n; ++i)
    printf("%02x", p[i]);
}
static int oracle(void) {
  struct rpm_counter state[3], next;
  char op;
  unsigned a, b;
  uint64_t amount;
  unsigned char wirebytes[160], packet[64];
  int status = 0;
  int64_t value;
  struct rpm_permit p;
  while (scanf(" %c %u %u %" SCNu64, &op, &a, &b, &amount) == 4) {
    CHECK(a < 3);
    if (op == 'I') {
      unsigned i;
      for (i = 0; i < 3; ++i)
        CHECK(rpm_make(9, 10 * (i + 1), &state[i]) == RPM_OK);
      status = RPM_OK;
    } else if (op == 'A') {
      CHECK(amount <= UINT32_MAX);
      status = rpm_add(&state[a], (int)b, (uint32_t)amount, &next);
      if (status == RPM_OK)
        state[a] = next;
    } else if (op == 'R') {
      status = rpm_replace(&state[a], amount, &next);
      if (status == RPM_OK)
        state[a] = next;
    } else if (op == 'M') {
      CHECK(b < 3);
      status = rpm_merge(&state[a], &state[b], &next);
      if (status == RPM_OK)
        state[a] = next;
    } else
      return 2;
    CHECK(rpm_value(&state[a], &value) == RPM_OK);
    CHECK(rpm_encode(&state[a], wirebytes) == RPM_OK);
    p = permit(state[a].self);
    CHECK(rpm_seal(&state[a], p.key, packet) == RPM_OK);
    printf("%d %" PRId64 " ", status, value);
    printhex(wirebytes, 160);
    putchar(' ');
    printhex(packet, 64);
    putchar('\n');
  }
  return 0;
}
static int metrics(void) {
  printf("{\"counter_size\":%zu,\"permit_size\":%zu,\"component_bytes\":16,"
         "\"snapshot_bytes\":160,\"packet_bytes\":64,\"capacity\":6}\n",
         sizeof(struct rpm_counter), sizeof(struct rpm_permit));
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "membership_join"))
    return join() || laws();
  if (!strcmp(argv[1], "membership_wire"))
    return wire();
  if (!strcmp(argv[1], "membership_auth"))
    return auth();
  if (!strcmp(argv[1], "membership_replay"))
    return replay();
  if (!strcmp(argv[1], "oracle"))
    return oracle();
  if (!strcmp(argv[1], "metrics"))
    return metrics();
  return 2;
}
