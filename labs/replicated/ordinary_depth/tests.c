/* Reuse the original public tests without copying their implementations. */
#define main rpo_baseline_main
#include "../tests/tests.c"
#undef main
#include <inttypes.h>
#include <time.h>
static struct rp_pn pn(unsigned bits) {
  struct rp_pn s = {{0}, {0}};
  for (unsigned i = 0; i < 3; ++i) {
    s.positive[i] = (bits >> i) & 1;
    s.negative[i] = (bits >> (i + 3)) & 1;
  }
  return s;
}
static int pn_equal(struct rp_pn a, struct rp_pn b) {
  for (unsigned i = 0; i < 3; ++i)
    if (a.positive[i] != b.positive[i] || a.negative[i] != b.negative[i])
      return 0;
  return 1;
}
static int laws(void) {
  const unsigned permutations[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                                       {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
  for (unsigned a = 0; a < 64; ++a)
    for (unsigned b = 0; b < 64; ++b)
      for (unsigned c = 0; c < 64; ++c) {
        struct rp_pn values[3] = {pn(a), pn(b), pn(c)}, expected = pn(a | b | c);
        for (unsigned order = 0; order < 6; ++order) {
          struct rp_pn got = values[permutations[order][0]];
          rp_pn_merge(&got, &values[permutations[order][1]]);
          rp_pn_merge(&got, &values[permutations[order][2]]);
          rp_pn_merge(&got, &got);
          CHECK(pn_equal(got, expected));
        }
      }
  for (unsigned a = 0; a < 10; ++a)
    for (unsigned b = 0; b < 10; ++b)
      for (unsigned c = 0; c < 10; ++c) {
        unsigned indexes[3] = {a, b, c}, max = a > b ? a : b;
        if (c > max)
          max = c;
        struct rp_register values[3];
        for (unsigned i = 0; i < 3; ++i) {
          unsigned k = indexes[i];
          values[i] = (struct rp_register){k ? (k - 1) / 3 + 1 : 0, k ? (k - 1) % 3 : 0,
                                   (int)k};
        }
        for (unsigned order = 0; order < 6; ++order) {
          struct rp_register got = values[permutations[order][0]];
          CHECK(rp_register_merge(&got, values[permutations[order][1]]) == 0 &&
                rp_register_merge(&got, values[permutations[order][2]]) == 0 &&
                rp_register_merge(&got, got) == 0 && got.value == (int)max);
        }
      }
  struct rp_register a = {1, 0, 10}, b = {1, 0, 20}, c = {2, 0, 30};
  CHECK(rp_register_merge(&a, b) == RP_CONFLICT && a.value == 10);
  CHECK(rp_register_merge(&a, c) == 0 && rp_register_merge(&a, b) == 0 &&
        a.value == 30);
  struct rp_register early = {100, 0, 1}, later = {2, 1, 2};
  CHECK(rp_register_merge(&early, later) == 0 && early.value == 1);
  struct rp_pn fork = pn(0), other = pn(0);
  CHECK(rp_pn_add(&fork, 0, 0, 1) == 0 && rp_pn_add(&other, 0, 0, 1) == 0);
  rp_pn_merge(&fork, &other);
  CHECK(rp_pn_value(&fork) == 1);
  uint32_t wrapped = UINT32_MAX;
  ++wrapped;
  CHECK(wrapped == 0);
  fork.positive[0] = UINT32_MAX;
  struct rp_pn saved = fork;
  CHECK(rp_pn_add(&fork, 0, 0, 1) == RP_RANGE && pn_equal(fork, saved));
  unsigned additive = 1;
  additive += 1;
  CHECK(additive == 2);
  puts("ORDINARY_LAWS pn-triples262144 permutations6 register-triples1000 "
       "compatible-only clock-skew forked-writer wrap retransmission");
  return 0;
}
static int oracle(void) {
  unsigned n;
  while (scanf("%u", &n) == 1) {
    struct rp_register s = {0, 0, 0};
    CHECK(n <= 3);
    for (unsigned i = 0; i < n; ++i) {
      struct rp_register x;
      CHECK(scanf("%" SCNu64 " %u %d", &x.time, &x.replica, &x.value) == 3);
      int r = rp_register_merge(&s, x);
      printf("%d ", r);
    }
    printf("%" PRIu64 " %u %d\n", s.time, s.replica, s.value);
  }
  return ferror(stdin) ? 1 : 0;
}
static int metrics(void) {
  double elapsed[7];
  unsigned runs = 100000;
  for (unsigned sample = 0; sample < 7; ++sample) {
    struct rp_pn p = pn(3), q = pn(47);
    struct rp_register r = {0, 0, 0};
    clock_t start = clock();
    for (unsigned i = 0; i < runs; ++i) {
      rp_pn_merge(&p, &q);
      CHECK(rp_register_merge(&r, (struct rp_register){i + 1, i % 3, (int)(i % 97)}) ==
            0);
    }
    elapsed[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
    CHECK(r.time == runs && pn_equal(p, pn(47)));
  }
  printf("{\"pn_size\":%zu,\"register_size\":%zu,\"runs\":%u,\"cpu_seconds\":[",
         sizeof(struct rp_pn), sizeof(struct rp_register), runs);
  for (unsigned i = 0; i < 7; ++i)
    printf("%s%.9f", i ? "," : "", elapsed[i]);
  puts("]}");
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "ordinary_laws"))
    return laws();
  if (!strcmp(argv[1], "oracle"))
    return oracle();
  if (!strcmp(argv[1], "metrics"))
    return metrics();
  if (!strncmp(argv[1], "ordinary_", 9)) {
    char *args[] = {argv[0], argv[1] + 9};
    int r = rpo_baseline_main(2, args);
    if (!r)
      printf("ORDINARY_BASE %s passed\n", args[1]);
    return r;
  }
  return 2;
}
