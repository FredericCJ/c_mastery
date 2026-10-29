#include "flags_negotiation.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "ADO_CHECK %s:%d: %s\n", __FILE__, __LINE__, #x);        \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int same(struct adf_agreement a, struct adf_agreement b) {
  return a.common == b.common && a.required == b.required &&
         a.advisory == b.advisory && a.enabled == b.enabled;
}
static int negotiate(void) {
  struct adf_agreement r = {77, 78, 79, 80}, old = r;
  struct adf_capabilities a = {1, 7}, b = {2, 63};
  CHECK(adf_negotiate(a, b, (struct adf_offer){1, 8},
                      (struct adf_offer){2, UINT32_C(0x80000004)}, &r) == ADF_OK);
  CHECK(same(r, (struct adf_agreement){7, 3, UINT32_C(0x8000000c), 7}));
  struct adf_agreement reversed;
  CHECK(adf_negotiate(b, a, (struct adf_offer){2, UINT32_C(0x80000004)},
                      (struct adf_offer){1, 8}, &reversed) == ADF_OK &&
        same(r, reversed));
  r = old;
  /* The counter is a harmless local unsupported-operation marker. */
  unsigned counter = 0;
  enum adf_status status =
      adf_negotiate(a, b, (struct adf_offer){0, 0}, (struct adf_offer){8, 0}, &r);
  if (status == ADF_OK && (r.enabled & UINT32_C(8)))
    ++counter;
  CHECK(counter == 0 && status == ADF_UNSUPPORTED && same(r, old));
  CHECK(adf_negotiate(a, b, (struct adf_offer){0, UINT32_MAX}, (struct adf_offer){0, 0}, &r) ==
        ADF_OK);
  CHECK(same(r, (struct adf_agreement){7, 0, UINT32_MAX, 7}));
  CHECK(adf_negotiate((struct adf_capabilities){2, 63}, b, (struct adf_offer){32, 0},
                      (struct adf_offer){0, 0}, &r) == ADF_OK &&
        r.enabled == 32);
  CHECK(adf_negotiate(a, b, (struct adf_offer){0, 0}, (struct adf_offer){0, 32}, &r) ==
            ADF_OK &&
        r.advisory == 32 && r.enabled == 0);
  struct adf_offer forwarded = {r.required, r.advisory};
  CHECK(adf_negotiate(b, b, forwarded, (struct adf_offer){0, 0}, &r) == ADF_OK &&
        r.advisory == 32 && r.enabled == 32);
  r = old;
  CHECK(adf_negotiate(a, b, (struct adf_offer){32, 0}, (struct adf_offer){0, 0}, &r) ==
            ADF_UNSUPPORTED &&
        same(r, old));
  r = old;
  CHECK(adf_negotiate(a, b, (struct adf_offer){UINT32_C(0x80000000), 0},
                      (struct adf_offer){0, 0}, &r) == ADF_UNSUPPORTED &&
        same(r, old));
  CHECK(adf_negotiate((struct adf_capabilities){1, 8}, b, (struct adf_offer){0, 0},
                      (struct adf_offer){0, 0}, &r) == ADF_INVALID &&
        same(r, old));
  CHECK(adf_negotiate((struct adf_capabilities){2, 64}, b, (struct adf_offer){0, 0},
                      (struct adf_offer){0, 0}, &r) == ADF_INVALID &&
        same(r, old));
  CHECK(adf_negotiate((struct adf_capabilities){3, 0}, b, (struct adf_offer){0, 0},
                      (struct adf_offer){0, 0}, &r) == ADF_INVALID &&
        same(r, old));
  CHECK(adf_negotiate(a, b, (struct adf_offer){0, 0}, (struct adf_offer){0, 0}, NULL) ==
        ADF_INVALID);
  CHECK(adf_negotiate((struct adf_capabilities){1, 0}, (struct adf_capabilities){2, 0},
                      (struct adf_offer){0, 0}, (struct adf_offer){0, 0}, &r) == ADF_OK &&
        same(r, (struct adf_agreement){0, 0, 0, 0}));
  puts("ADF_NEGOTIATION intersection-authority advisory-forwarding "
       "unchanged-refusal");
  return 0;
}
static int sets(void) {
  for (uint32_t a = 0; a < 8; ++a)
    for (uint32_t b = 0; b < 8; ++b) {
      struct ad_flags x = {a}, y = {b}, r = {99}, u, i;
      CHECK(adf_union(x, y, &u) == ADF_OK && u.bits == (a | b));
      CHECK(adf_intersection(x, y, &i) == ADF_OK && i.bits == (a & b));
      CHECK(adf_intersection(x, u, &r) == ADF_OK && r.bits == a);
      CHECK(adf_union(x, i, &r) == ADF_OK && r.bits == a);
      CHECK(adf_union(x, y, &x) == ADF_OK && x.bits == (a | b));
      for (uint32_t c = 0; c < 8; ++c) {
        struct ad_flags left, right, mid;
        CHECK(adf_union(u, (struct ad_flags){c}, &left) == ADF_OK);
        CHECK(adf_union(y, (struct ad_flags){c}, &mid) == ADF_OK);
        CHECK(adf_union((struct ad_flags){a}, mid, &right) == ADF_OK &&
              left.bits == right.bits);
        CHECK(adf_intersection(i, (struct ad_flags){c}, &left) == ADF_OK);
        CHECK(adf_intersection(y, (struct ad_flags){c}, &mid) == ADF_OK);
        CHECK(adf_intersection((struct ad_flags){a}, mid, &right) == ADF_OK &&
              left.bits == right.bits);
      }
    }
  struct ad_flags r = {99};
  CHECK(adf_union((struct ad_flags){8}, (struct ad_flags){1}, &r) == ADF_INVALID &&
        r.bits == 99);
  CHECK(adf_intersection((struct ad_flags){1}, (struct ad_flags){UINT32_MAX}, &r) ==
            ADF_INVALID &&
        r.bits == 99);
  CHECK(adf_union((struct ad_flags){0}, (struct ad_flags){0}, NULL) == ADF_INVALID);
  CHECK(adf_intersection((struct ad_flags){0}, (struct ad_flags){0}, NULL) == ADF_INVALID);
  struct adf_capabilities cap = {99, 88};
  CHECK(adf_capabilities(2, 63, &cap) == ADF_OK && cap.version == 2 &&
        cap.bits == 63);
  CHECK(adf_capabilities(1, 0, &cap) == ADF_OK && cap.version == 1 &&
        cap.bits == 0);
  cap = (struct adf_capabilities){99, 88};
  CHECK(adf_capabilities(1, 8, &cap) == ADF_INVALID && cap.version == 99 &&
        cap.bits == 88);
  CHECK(adf_capabilities(2, 64, &cap) == ADF_INVALID && cap.version == 99 &&
        cap.bits == 88);
  CHECK(adf_capabilities(0, 0, &cap) == ADF_INVALID && cap.version == 99 &&
        cap.bits == 88);
  CHECK(adf_capabilities(1, 7, NULL) == ADF_INVALID);
  puts("ADF_SETS complete-three-bit-lattice validated-operands");
  return 0;
}
static int original(void) {
  for (uint32_t raw = 0; raw < 256; ++raw) {
    struct ad_flags out = {99};
    int ok = ad_flags(raw, &out);
    CHECK(ok == (raw < 8) && out.bits == (raw < 8 ? raw : 99));
  }
  for (unsigned k = 0; k < 32; ++k) {
    uint32_t raw = UINT32_C(1) << k;
    struct ad_flags out = {99};
    int ok = ad_flags(raw, &out);
    CHECK(ok == (k < 3) && out.bits == (k < 3 ? raw : 99));
  }
  struct ad_flags r = {99};
  CHECK(!ad_flags(UINT32_MAX, &r) && r.bits == 99);
  puts("ADF_BASE actual-ad-flags empty-reserved-high-bit");
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "flags_negotiation"))
    return negotiate();
  if (!strcmp(argv[1], "flags_sets"))
    return sets();
  if (!strcmp(argv[1], "flags_base"))
    return original();
  if (!strcmp(argv[1], "oracle")) {
    unsigned lv, pv;
    uint32_t lc, pc, lr, la, pr, pa;
    while (scanf("%u %" SCNu32 " %u %" SCNu32 " %" SCNu32 " %" SCNu32
                 " %" SCNu32 " %" SCNu32,
                 &lv, &lc, &pv, &pc, &lr, &la, &pr, &pa) == 8) {
      struct adf_agreement r = {77, 78, 79, 80};
      enum adf_status s =
          adf_negotiate((struct adf_capabilities){lv, lc}, (struct adf_capabilities){pv, pc},
                        (struct adf_offer){lr, la}, (struct adf_offer){pr, pa}, &r);
      printf("%u %" PRIu32 " %" PRIu32 " %" PRIu32 " %" PRIu32 "\n",
             (unsigned)s, r.common, r.required, r.advisory, r.enabled);
    }
    return 0;
  }
  if (!strcmp(argv[1], "metrics")) {
    double times[7];
    uint64_t checksum = 0;
    for (unsigned run = 0; run < 7; ++run) {
      clock_t start = clock();
      uint64_t sum = 0;
      for (unsigned i = 0; i < 100000; ++i) {
        struct adf_agreement r;
        CHECK(adf_negotiate((struct adf_capabilities){1, 7}, (struct adf_capabilities){2, 63},
                            (struct adf_offer){i % 8, UINT32_C(0x80000000)},
                            (struct adf_offer){0, 0}, &r) == ADF_OK);
        sum += r.enabled;
      }
      CHECK(sum == 350000);
      checksum = sum;
      times[run] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf("{\"capabilities_size\":%zu,\"offer_size\":%zu,\"agreement_size\":%"
           "zu,\"iterations\":100000,\"checksum\":%" PRIu64 ",\"seconds\":[",
           sizeof(struct adf_capabilities), sizeof(struct adf_offer), sizeof(struct adf_agreement),
           checksum);
    for (unsigned i = 0; i < 7; ++i)
      printf("%s%.9f", i ? "," : "", times[i]);
    puts("]}");
    return 0;
  }
  return 2;
}
