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
static int same(AdfAgreement a, AdfAgreement b) {
  return a.common == b.common && a.required == b.required &&
         a.advisory == b.advisory && a.enabled == b.enabled;
}
static int negotiate(void) {
  AdfAgreement r = {77, 78, 79, 80}, old = r;
  AdfCapabilities a = {1, 7}, b = {2, 63};
  CHECK(adf_negotiate(a, b, (AdfOffer){1, 8},
                      (AdfOffer){2, UINT32_C(0x80000004)}, &r) == ADF_OK);
  CHECK(same(r, (AdfAgreement){7, 3, UINT32_C(0x8000000c), 7}));
  AdfAgreement reversed;
  CHECK(adf_negotiate(b, a, (AdfOffer){2, UINT32_C(0x80000004)},
                      (AdfOffer){1, 8}, &reversed) == ADF_OK &&
        same(r, reversed));
  r = old;
  /* The counter is a harmless local unsupported-operation marker. */
  unsigned counter = 0;
  AdfStatus status =
      adf_negotiate(a, b, (AdfOffer){0, 0}, (AdfOffer){8, 0}, &r);
  if (status == ADF_OK && (r.enabled & UINT32_C(8)))
    ++counter;
  CHECK(counter == 0 && status == ADF_UNSUPPORTED && same(r, old));
  CHECK(adf_negotiate(a, b, (AdfOffer){0, UINT32_MAX}, (AdfOffer){0, 0}, &r) ==
        ADF_OK);
  CHECK(same(r, (AdfAgreement){7, 0, UINT32_MAX, 7}));
  CHECK(adf_negotiate((AdfCapabilities){2, 63}, b, (AdfOffer){32, 0},
                      (AdfOffer){0, 0}, &r) == ADF_OK &&
        r.enabled == 32);
  CHECK(adf_negotiate(a, b, (AdfOffer){0, 0}, (AdfOffer){0, 32}, &r) ==
            ADF_OK &&
        r.advisory == 32 && r.enabled == 0);
  AdfOffer forwarded = {r.required, r.advisory};
  CHECK(adf_negotiate(b, b, forwarded, (AdfOffer){0, 0}, &r) == ADF_OK &&
        r.advisory == 32 && r.enabled == 32);
  r = old;
  CHECK(adf_negotiate(a, b, (AdfOffer){32, 0}, (AdfOffer){0, 0}, &r) ==
            ADF_UNSUPPORTED &&
        same(r, old));
  r = old;
  CHECK(adf_negotiate(a, b, (AdfOffer){UINT32_C(0x80000000), 0},
                      (AdfOffer){0, 0}, &r) == ADF_UNSUPPORTED &&
        same(r, old));
  CHECK(adf_negotiate((AdfCapabilities){1, 8}, b, (AdfOffer){0, 0},
                      (AdfOffer){0, 0}, &r) == ADF_INVALID &&
        same(r, old));
  CHECK(adf_negotiate((AdfCapabilities){2, 64}, b, (AdfOffer){0, 0},
                      (AdfOffer){0, 0}, &r) == ADF_INVALID &&
        same(r, old));
  CHECK(adf_negotiate((AdfCapabilities){3, 0}, b, (AdfOffer){0, 0},
                      (AdfOffer){0, 0}, &r) == ADF_INVALID &&
        same(r, old));
  CHECK(adf_negotiate(a, b, (AdfOffer){0, 0}, (AdfOffer){0, 0}, NULL) ==
        ADF_INVALID);
  CHECK(adf_negotiate((AdfCapabilities){1, 0}, (AdfCapabilities){2, 0},
                      (AdfOffer){0, 0}, (AdfOffer){0, 0}, &r) == ADF_OK &&
        same(r, (AdfAgreement){0, 0, 0, 0}));
  puts("ADF_NEGOTIATION intersection-authority advisory-forwarding "
       "unchanged-refusal");
  return 0;
}
static int sets(void) {
  for (uint32_t a = 0; a < 8; ++a)
    for (uint32_t b = 0; b < 8; ++b) {
      ADFlags x = {a}, y = {b}, r = {99}, u, i;
      CHECK(adf_union(x, y, &u) == ADF_OK && u.bits == (a | b));
      CHECK(adf_intersection(x, y, &i) == ADF_OK && i.bits == (a & b));
      CHECK(adf_intersection(x, u, &r) == ADF_OK && r.bits == a);
      CHECK(adf_union(x, i, &r) == ADF_OK && r.bits == a);
      CHECK(adf_union(x, y, &x) == ADF_OK && x.bits == (a | b));
      for (uint32_t c = 0; c < 8; ++c) {
        ADFlags left, right, mid;
        CHECK(adf_union(u, (ADFlags){c}, &left) == ADF_OK);
        CHECK(adf_union(y, (ADFlags){c}, &mid) == ADF_OK);
        CHECK(adf_union((ADFlags){a}, mid, &right) == ADF_OK &&
              left.bits == right.bits);
        CHECK(adf_intersection(i, (ADFlags){c}, &left) == ADF_OK);
        CHECK(adf_intersection(y, (ADFlags){c}, &mid) == ADF_OK);
        CHECK(adf_intersection((ADFlags){a}, mid, &right) == ADF_OK &&
              left.bits == right.bits);
      }
    }
  ADFlags r = {99};
  CHECK(adf_union((ADFlags){8}, (ADFlags){1}, &r) == ADF_INVALID &&
        r.bits == 99);
  CHECK(adf_intersection((ADFlags){1}, (ADFlags){UINT32_MAX}, &r) ==
            ADF_INVALID &&
        r.bits == 99);
  CHECK(adf_union((ADFlags){0}, (ADFlags){0}, NULL) == ADF_INVALID);
  CHECK(adf_intersection((ADFlags){0}, (ADFlags){0}, NULL) == ADF_INVALID);
  AdfCapabilities cap = {99, 88};
  CHECK(adf_capabilities(2, 63, &cap) == ADF_OK && cap.version == 2 &&
        cap.bits == 63);
  CHECK(adf_capabilities(1, 0, &cap) == ADF_OK && cap.version == 1 &&
        cap.bits == 0);
  cap = (AdfCapabilities){99, 88};
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
    ADFlags out = {99};
    int ok = ad_flags(raw, &out);
    CHECK(ok == (raw < 8) && out.bits == (raw < 8 ? raw : 99));
  }
  for (unsigned k = 0; k < 32; ++k) {
    uint32_t raw = UINT32_C(1) << k;
    ADFlags out = {99};
    int ok = ad_flags(raw, &out);
    CHECK(ok == (k < 3) && out.bits == (k < 3 ? raw : 99));
  }
  ADFlags r = {99};
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
      AdfAgreement r = {77, 78, 79, 80};
      AdfStatus s =
          adf_negotiate((AdfCapabilities){lv, lc}, (AdfCapabilities){pv, pc},
                        (AdfOffer){lr, la}, (AdfOffer){pr, pa}, &r);
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
        AdfAgreement r;
        CHECK(adf_negotiate((AdfCapabilities){1, 7}, (AdfCapabilities){2, 63},
                            (AdfOffer){i % 8, UINT32_C(0x80000000)},
                            (AdfOffer){0, 0}, &r) == ADF_OK);
        sum += r.enabled;
      }
      CHECK(sum == 350000);
      checksum = sum;
      times[run] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf("{\"capabilities_size\":%zu,\"offer_size\":%zu,\"agreement_size\":%"
           "zu,\"iterations\":100000,\"checksum\":%" PRIu64 ",\"seconds\":[",
           sizeof(AdfCapabilities), sizeof(AdfOffer), sizeof(AdfAgreement),
           checksum);
    for (unsigned i = 0; i < 7; ++i)
      printf("%s%.9f", i ? "," : "", times[i]);
    puts("]}");
    return 0;
  }
  return 2;
}
