#include "services.h"
#include "store_host.h"
#include <limits.h>
#ifndef VH_MUTANT
#define VH_MUTANT 0
#endif
static const char value_key[] = "vm-result", receipt_key[] = "vm-operation";
static int ready(const struct vg_vh *h) {
  return h && h->database && h->mode <= VH_VERSIONED5 && h->allow_legacy <= 1;
}
static void put32(uint8_t *p, int32_t n) {
  uint32_t u = (uint32_t)n;
  for (unsigned i = 0; i < 4; i++) {
    p[i] = (uint8_t)u;
    u >>= 8;
  }
}
static int32_t get32(const uint8_t *p) {
  uint32_t u = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
               ((uint32_t)p[3] << 24);
  return u <= INT32_MAX ? (int32_t)u : -1 - (int32_t)(UINT32_MAX - u);
}
static void put64(uint8_t *p, uint64_t u) {
  for (unsigned i = 0; i < 8; i++) {
    p[i] = (uint8_t)u;
    u >>= 8;
  }
}
static uint64_t get64(const uint8_t *p) {
  uint64_t u = 0;
  for (unsigned i = 0; i < 8; i++)
    u |= (uint64_t)p[i] << (8 * i);
  return u;
}
static int read_value(struct vg_vh *h, int32_t *out, int *legacy) {
  uint8_t bytes[STORE_VALUE_MAX] = {0};
  size_t length = 0;
  int r = store_get(h->database, value_key, sizeof value_key - 1, bytes,
                    sizeof bytes, &length);
  if (r)
    return r;
  if (length == 4 && h->allow_legacy) {
    *out = get32(bytes);
    *legacy = 1;
    return STORE_OK;
  }
  if (VH_MUTANT != 2 && (length != 5 || bytes[0] != 1))
    return STORE_CORRUPT;
  *out = get32(bytes + 1);
  *legacy = 0;
  return STORE_OK;
}
static int put_value(store *s, int32_t value) {
  uint8_t bytes[5] = {1};
  put32(bytes + 1, value);
  return store_put(s, value_key, sizeof value_key - 1, bytes, sizeof bytes);
}
static int commit(store *s) {
  int r = store_commit(s);
  return VH_MUTANT == 4 ? STORE_OK : r;
}
static int assign(struct vg_vh *h, int32_t value, int32_t *out) {
  int r = store_begin(h->database);
  if (r)
    return r;
  r = put_value(h->database, value);
  if (r) {
    store_abort(h->database);
    return r;
  }
  r = commit(h->database);
  if (!r)
    *out = value;
  return r;
}
int vh_migrate(struct vg_vh *h, int32_t *out) {
  if (!ready(h) || !out || h->mode != VH_VERSIONED5 || !h->allow_legacy)
    return STORE_INVALID;
  int32_t value;
  int legacy;
  int r = read_value(h, &value, &legacy);
  if (r)
    return r;
  if (legacy)
    return assign(h, value, out);
  *out = value;
  return STORE_OK;
}
int vh_increment(struct vg_vh *h, uint64_t id, int32_t delta, int32_t *out) {
  if (!ready(h) || !out || h->mode != VH_VERSIONED5 || !id)
    return STORE_INVALID;
  uint8_t receipt[STORE_VALUE_MAX] = {0};
  size_t n = 0;
  uint64_t previous = 0;
  int r = store_get(h->database, receipt_key, sizeof receipt_key - 1, receipt,
                    sizeof receipt, &n);
  if (r != STORE_OK && r != STORE_NOT_FOUND)
    return r;
  if (r == STORE_OK) {
    if (n != 17 || receipt[0] != 1)
      return STORE_CORRUPT;
    previous = get64(receipt + 1);
    if (!previous)
      return STORE_CORRUPT;
    if (id == previous && VH_MUTANT != 3) {
      if (get32(receipt + 9) != delta)
        return STORE_INVALID;
      *out = get32(receipt + 13);
      return STORE_OK;
    }
  }
  if (!(VH_MUTANT == 3 && id == previous) &&
      (previous == UINT64_MAX || id != previous + 1))
    return STORE_INVALID;
  int32_t value;
  int legacy;
  r = read_value(h, &value, &legacy);
  if (r)
    return r;
  int64_t sum = (int64_t)value + delta;
  if (sum < INT32_MIN || sum > INT32_MAX)
    return STORE_INVALID;
  value = (int32_t)sum;
  receipt[0] = 1;
  put64(receipt + 1, id);
  put32(receipt + 9, delta);
  put32(receipt + 13, value);
  r = store_begin(h->database);
  if (r)
    return r;
  r = put_value(h->database, value);
  if (!r && VH_MUTANT != 5)
    r = store_put(h->database, receipt_key, sizeof receipt_key - 1, receipt,
                  17);
  if (r) {
    store_abort(h->database);
    return r;
  }
  r = commit(h->database);
  if (!r)
    *out = value;
  return r;
}
int vh_host(void *context, uint8_t service, int32_t input, int32_t *out) {
  struct vg_vh *h = context;
  if (!ready(h) || !out)
    return STORE_INVALID;
  if (VH_MUTANT != 1 &&
      (service >= 32 || !(h->capabilities & (UINT32_C(1) << service))))
    return STORE_INVALID;
  if (h->mode == VH_LEGACY4) {
    if (service != 1 && service != 2)
      return STORE_INVALID;
    return cvm_store_host(h->database, service, input, out);
  }
  if (service == 1)
    return assign(h, input, out);
  if (service == 2) {
    int32_t value;
    int legacy;
    int r = read_value(h, &value, &legacy);
    if (!r)
      *out = value;
    return r;
  }
  if (service == 3)
    return vh_increment(h, h->operation_id, input, out);
  return STORE_INVALID;
}
