#include "hashing.h"
#include "merkle_repair.h"
#include "replicated.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x);          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct rp_peer {
  struct rpk_tree tree;
  unsigned calls, fail, mode, chunk, cut, tamper_at;
};
static struct rpk_tree make(unsigned n) {
  struct rpk_tree t = {0};
  unsigned i;
  t.domain = 7;
  t.count = n;
  for (i = 0; i < n; ++i) {
    t.leaves[i].length = 4;
    t.leaves[i].bytes[3] = (unsigned char)(i + 1);
  }
  return t;
}
static unsigned at(unsigned raw) { return raw / 8 * 9 + 1 + raw % 8; }
static int exchange(void *context, const unsigned char *q, size_t qn,
                    unsigned char *reply, size_t capacity, size_t *length) {
  struct rp_peer *p = context;
  int r;
  ++p->calls;
  if (p->calls == p->fail)
    return 1;
  r = rpk_serve(&p->tree, q, qn, p->chunk, reply, capacity, length);
  if (r)
    return 1;
  if (p->mode == 1)
    reply[at(23)] ^= 1;
  if (p->mode == 2)
    reply[at(26)] = 1;
  if (p->mode == 3 && (!p->tamper_at || p->calls == p->tamper_at))
    reply[at(32)] ^= 1;
  if (p->mode == 4)
    --*length;
  if (p->mode == 5)
    reply[0] = 9;
  if (p->mode == 6)
    *length = 109;
  if (p->mode == 7)
    *length = p->cut;
  return 0;
}
static int proof(void) {
  struct rpk_tree t;
  struct rpk_commitment root, other;
  struct rpk_proof p, bad;
  unsigned n, i, j;
  unsigned char raw[32], bound[41], swap[32];
  uint32_t values[8];
  for (n = 1; n <= 16; ++n) {
    t = make(n);
    CHECK(rpk_commit(&t, &root) == RPK_OK);
    for (i = 0; i < n; ++i) {
      CHECK(rpk_prove(&t, i, &p) == RPK_OK);
      CHECK(rpk_verify(&t.leaves[i], &p, &root) == RPK_OK);
      if (p.depth) {
        bad = p;
        --bad.depth;
        CHECK(rpk_verify(&t.leaves[i], &bad, &root) == RPK_PROOF);
        for (j = 0; j < p.depth; ++j) {
          bad = p;
          bad.siblings[j][0] ^= 1;
          CHECK(rpk_verify(&t.leaves[i], &bad, &root) == RPK_PROOF);
        }
      }
      bad = p;
      bad.index += n;
      CHECK(rpk_verify(&t.leaves[i], &bad, &root) == RPK_PROOF);
      if (p.depth >= 2) {
        bad = p;
        memcpy(swap, bad.siblings[0], 32);
        memcpy(bad.siblings[0], bad.siblings[1], 32);
        memcpy(bad.siblings[1], swap, 32);
        CHECK(rpk_verify(&t.leaves[i], &bad, &root) == RPK_PROOF);
      }
      bad = p;
      bad.count = n == 16 ? 15 : n + 1;
      CHECK(rpk_verify(&t.leaves[i], &bad, &root) == RPK_PROOF);
      bad = p;
      bad.depth = 5;
      CHECK(rpk_verify(&t.leaves[i], &bad, &root) == RPK_INVALID);
      if (p.depth < 4) {
        bad = p;
        ++bad.depth;
        CHECK(rpk_verify(&t.leaves[i], &bad, &root) == RPK_PROOF);
      }
      other = root;
      ++other.domain;
      CHECK(rpk_verify(&t.leaves[i], &p, &other) == RPK_PROOF);
      other = root;
      other.count = n == 16 ? 15 : n + 1;
      CHECK(rpk_verify(&t.leaves[i], &p, &other) == RPK_PROOF);
    }
    bad = p;
    CHECK(rpk_prove(&t, n, &p) == RPK_INVALID && !memcmp(&p, &bad, sizeof p));
  }
  t = make(0);
  memset(&p, 0xa5, sizeof p);
  memcpy(&bad, &p, sizeof p);
  CHECK(rpk_prove(&t, 0, &p) == RPK_INVALID && !memcmp(&p, &bad, sizeof p));
  for (n = 0; n <= 8; ++n) {
    t = make(n);
    for (i = 0; i < n; ++i)
      values[i] = i + 1;
    CHECK(rp_merkle(values, n, raw) == RP_OK);
    memset(bound, 0, sizeof bound);
    bound[0] = 2;
    bound[4] = 7;
    bound[8] = (unsigned char)n;
    memcpy(bound + 9, raw, 32);
    CHECK(rpk_commit(&t, &root) == RPK_OK);
    CHECK(rph_hash(bound, 41, raw) == RPH_OK && rph_equal(raw, root.root));
  }
  t = make(1);
  t.leaves[0].length = 2;
  t.leaves[0].bytes[0] = 'a';
  t.leaves[0].bytes[1] = 'b';
  CHECK(rpk_commit(&t, &root) == RPK_OK);
  t.count = 2;
  t.leaves[0].length = 1;
  t.leaves[1].length = 1;
  t.leaves[1].bytes[0] = 'b';
  CHECK(rpk_commit(&t, &other) == RPK_OK && !rph_equal(root.root, other.root));
  puts("MERKLE_PROOFS leaves136 index-count-path domains chunk-boundaries");
  puts("MERKLE_MALFORMED reordered-siblings mismatched-count depth5 "
       "empty-proof unchanged-output");
  return 0;
}
static int wire(void) {
  struct rp_peer p = {0};
  struct rpk_tree t, saved;
  struct rpk_commitment root;
  struct rpk_cost cost, before;
  unsigned chunk, mode;
  p.tree = make(5);
  CHECK(rpk_commit(&p.tree, &root) == RPK_OK);
  for (chunk = 1; chunk <= 108; ++chunk) {
    t = make(5);
    t.leaves[0].bytes[0] = 7;
    p.calls = 0;
    p.chunk = chunk;
    p.mode = 0;
    CHECK(rpk_repair(&t, &root, chunk, 0, exchange, &p, &cost) == RPK_OK);
    CHECK(t.leaves[0].bytes[0] == 0);
  }
  for (mode = 1; mode <= 6; ++mode) {
    t = make(5);
    t.leaves[0].bytes[0] = 7;
    saved = t;
    p.calls = 0;
    p.mode = mode;
    p.chunk = 3;
    memset(&cost, 0xa5, sizeof cost);
    memcpy(&before, &cost, sizeof cost);
    CHECK(rpk_repair(&t, &root, 7, 0, exchange, &p, &cost) != RPK_OK &&
          !memcmp(&t, &saved, sizeof t) &&
          !memcmp(&cost, &before, sizeof cost));
  }
  p.mode = 7;
  for (p.cut = 0; p.cut < 108; ++p.cut) {
    t = make(5);
    t.leaves[0].bytes[0] = 7;
    saved = t;
    CHECK(rpk_repair(&t, &root, 3, 0, exchange, &p, &cost) == RPK_PROTOCOL &&
          !memcmp(&t, &saved, sizeof t));
  }
  puts("MERKLE_WIRE chunks108 truncations108 nonce padding missing-byte "
       "malformed-length "
       "oversized-reply");
  return 0;
}
static int repair(void) {
  struct rp_peer p = {0};
  struct rpk_tree t, saved;
  struct rpk_commitment root, actual;
  struct rpk_cost cost, before;
  unsigned n, fail;
  p.tree = make(2);
  p.chunk = 1;
  CHECK(rpk_commit(&p.tree, &root) == RPK_OK);
  for (fail = 1; fail <= 3; ++fail) {
    t = make(2);
    t.leaves[0].bytes[0] = 9;
    t.leaves[1].bytes[0] = 9;
    saved = t;
    p.calls = 0;
    p.fail = fail;
    memset(&cost, 0xa5, sizeof cost);
    memcpy(&before, &cost, sizeof cost);
    CHECK(rpk_repair(&t, &root, 1, 0, exchange, &p, &cost) == RPK_MISSING &&
          !memcmp(&t, &saved, sizeof t) &&
          !memcmp(&cost, &before, sizeof cost));
  }
  p.fail = 0;
  p.mode = 3;
  p.calls = 0;
  t = make(2);
  t.leaves[0].bytes[0] = 9;
  saved = t;
  CHECK(rpk_repair(&t, &root, 2, 0, exchange, &p, &cost) == RPK_PROOF &&
        !memcmp(&t, &saved, sizeof t));
  p.calls = 0;
  p.tamper_at = 2;
  CHECK(rpk_repair(&t, &root, 2, 0, exchange, &p, &cost) == RPK_PROOF &&
        !memcmp(&t, &saved, sizeof t));
  p.mode = 0;
  for (n = 0; n <= 16; ++n) {
    p.tree = make(n);
    p.calls = 0;
    CHECK(rpk_commit(&p.tree, &root) == RPK_OK);
    t = make(16 - n);
    ++t.domain;
    CHECK(rpk_repair(&t, &root, 5, 0, exchange, &p, &cost) == RPK_OK);
    CHECK(rpk_commit(&t, &actual) == RPK_OK &&
          rph_equal(actual.root, root.root) && t.count == n && t.domain == 7);
  }
  puts("MERKLE_REPAIR failed-node-no-publication counts0through16");
  return 0;
}
static int cost(void) {
  struct rp_peer p = {0};
  struct rpk_tree t;
  struct rpk_commitment root;
  struct rpk_cost sparse, full, equal, direct;
  p.tree = make(16);
  p.chunk = 8;
  CHECK(rpk_commit(&p.tree, &root) == RPK_OK);
  t = p.tree;
  CHECK(rpk_repair(&t, &root, 8, 0, exchange, &p, &equal) == RPK_OK &&
        !equal.requests);
  t = p.tree;
  t.leaves[7].bytes[0] = 9;
  p.calls = 0;
  CHECK(rpk_repair(&t, &root, 8, 0, exchange, &p, &sparse) == RPK_OK &&
        sparse.requests == 5 && sparse.leaves_received == 1);
  t = p.tree;
  t.leaves[7].bytes[0] = 9;
  p.calls = 0;
  CHECK(rpk_repair(&t, &root, 8, 1, exchange, &p, &full) == RPK_OK &&
        full.requests == 31 && full.leaves_received == 16);
  t = p.tree;
  t.leaves[7].bytes[0] = 9;
  p.calls = 0;
  CHECK(rpk_repair(&t, &root, 8, 2, exchange, &p, &direct) == RPK_OK &&
        direct.requests == 16 && direct.leaves_received == 16);
  printf("MERKLE_DIRECT requests%u bytes%u\n", direct.requests,
         direct.request_bytes + direct.response_bytes);
  printf("MERKLE_COST equal0 sparse%u full%u sparse-bytes%u full-bytes%u\n",
         sparse.requests, full.requests,
         sparse.request_bytes + sparse.response_bytes,
         full.request_bytes + full.response_bytes);
  return 0;
}
static int oracle(void) {
  struct rpk_tree t;
  struct rpk_commitment root;
  struct rpk_proof p;
  unsigned domain, n, index, i, j, l, b;
  while (scanf("%u %u %u", &domain, &n, &index) == 3) {
    CHECK(n <= 16);
    memset(&t, 0, sizeof t);
    t.domain = domain;
    t.count = n;
    for (i = 0; i < n; ++i) {
      CHECK(scanf("%u", &l) == 1 && l <= 32);
      t.leaves[i].length = l;
      for (j = 0; j < l; ++j) {
        CHECK(scanf("%u", &b) == 1 && b <= 255);
        t.leaves[i].bytes[j] = (unsigned char)b;
      }
    }
    CHECK(rpk_commit(&t, &root) == RPK_OK);
    for (i = 0; i < 32; ++i)
      printf("%02x", root.root[i]);
    if (n) {
      CHECK(rpk_prove(&t, index, &p) == RPK_OK);
      CHECK(rpk_verify(&t.leaves[index], &p, &root) == RPK_OK);
      printf(" %u", p.depth);
      for (i = 0; i < p.depth; ++i) {
        putchar(' ');
        for (j = 0; j < 32; ++j)
          printf("%02x", p.siblings[i][j]);
      }
    } else
      printf(" 0");
    puts("");
  }
  return 0;
}

static int repair_oracle(void) {
  unsigned n, mask, chunk, full, fail, mode, i;
  while (scanf("%u %u %u %u %u %u", &n, &mask, &chunk, &full, &fail, &mode) ==
         6) {
    struct rp_peer peer = {0};
    struct rpk_tree t;
    struct rpk_commitment root, actual;
    struct rpk_cost cost = {0};
    int status;
    CHECK(n <= 16);
    peer.tree = make(n);
    peer.chunk = chunk;
    peer.fail = fail;
    peer.mode = mode;
    t = peer.tree;
    for (i = 0; i < n; ++i)
      if (mask & (1u << i))
        t.leaves[i].bytes[0] = 9;
    CHECK(rpk_commit(&peer.tree, &root) == RPK_OK);
    status = rpk_repair(&t, &root, chunk, full, exchange, &peer, &cost);
    CHECK(rpk_commit(&t, &actual) == RPK_OK);
    printf("%d ", status);
    for (i = 0; i < 32; ++i)
      printf("%02x", actual.root[i]);
    printf(" %u %u %u %u\n", cost.requests, cost.leaves_received,
           cost.request_bytes, cost.response_bytes);
  }
  return 0;
}
static int metrics(void) {
  struct rp_peer p = {0};
  struct rpk_tree t;
  struct rpk_commitment root;
  struct rpk_cost observed[3];
  clock_t start, end;
  double seconds[3];
  unsigned full, i;
  p.tree = make(16);
  p.chunk = 8;
  CHECK(rpk_commit(&p.tree, &root) == RPK_OK);
  for (full = 0; full < 3; ++full) {
    start = clock();
    CHECK(start != (clock_t)-1);
    for (i = 0; i < 200; ++i) {
      t = p.tree;
      t.leaves[7].bytes[0] = 9;
      p.calls = 0;
      CHECK(rpk_repair(&t, &root, 8, full, exchange, &p, &observed[full]) ==
            RPK_OK);
    }
    end = clock();
    CHECK(end != (clock_t)-1 && end >= start);
    seconds[full] = (double)(end - start) / CLOCKS_PER_SEC;
  }
  printf("{\"tree_size\":%zu,\"proof_size\":%zu,\"cost_size\":%zu,\"max_"
         "leaves\":16,\"leaf_bytes\":32,\"request_wire\":27,\"response_wire\":"
         "108,\"runs\":200,\"sparse_hashes\":%u,\"full_hashes\":%u,\"sparse_"
         "requests\":%u,\"full_requests\":%u,\"sparse_cpu_seconds\":%.9f,"
         "\"full_cpu_seconds\":%.9f,\"direct_hashes\":%u,\"direct_requests\":%"
         "u,\"direct_cpu_seconds\":%.9f}\n",
         sizeof(struct rpk_tree), sizeof(struct rpk_proof), sizeof(struct rpk_cost), observed[0].hashes,
         observed[1].hashes, observed[0].requests, observed[1].requests,
         seconds[0], seconds[1], observed[2].hashes, observed[2].requests,
         seconds[2]);
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "merkle_proof"))
    return proof();
  if (!strcmp(argv[1], "merkle_wire"))
    return wire();
  if (!strcmp(argv[1], "merkle_repair"))
    return repair();
  if (!strcmp(argv[1], "merkle_cost"))
    return cost();
  if (!strcmp(argv[1], "oracle"))
    return oracle();
  if (!strcmp(argv[1], "repair_oracle"))
    return repair_oracle();
  if (!strcmp(argv[1], "metrics"))
    return metrics();
  return 2;
}
