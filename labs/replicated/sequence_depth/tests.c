#include "hashing.h"
#include "sequence_depth.h"
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
static int tree(void) {
  struct rps_tree t, saved;
  unsigned mode, i;
  unsigned char out[64], want[] = {'b', 'a', 0xf0, 0x9f, 0x98, 0x80};
  size_t n;
  for (mode = 0; mode < 2; ++mode) {
    CHECK(rps_init(&t, 1, mode) == RPS_OK);
    CHECK(rps_remove(&t, 1, 2) == RPS_OK);
    CHECK(rps_insert(&t, 1, 3, 2, 0x1f600) == RPS_OK);
    CHECK(rps_insert(&t, 1, 2, 1, 0x20ac) == RPS_OK);
    CHECK(rps_render(&t, out, 64, &n) == RPS_OK && n == 0);
    CHECK(rps_insert(&t, 1, 1, 0, 'a') == RPS_OK);
    CHECK(rps_insert(&t, 1, 4, 0, 'b') == RPS_OK);
    CHECK(rps_render(&t, out, 64, &n) == RPS_OK && n == sizeof want &&
          !memcmp(out, want, n));
    saved = t;
    CHECK(rps_insert(&t, 1, 5, 0, 0xd800) == RPS_INVALID &&
          !memcmp(&t, &saved, sizeof t));
    CHECK(rps_insert(&t, 1, 5, 0, 0x110000) == RPS_INVALID);
    CHECK(rps_insert(&t, 1, 3, 2, 'x') == RPS_CONFLICT);
    memset(out, 0xa5, sizeof out);
    n = 77;
    CHECK(rps_render(&t, out, 5, &n) == RPS_FULL && n == 77 && out[0] == 0xa5);
    CHECK(rps_insert(&t, 1, 5, 0, 0x10ffff) == RPS_OK);
    CHECK(rps_insert(&t, 1, 6, 0, 0) == RPS_OK);
    CHECK(rps_render(&t, out, 64, &n) == RPS_OK && out[0] == 0 &&
          out[1] == 0xf4 && out[2] == 0x8f && out[3] == 0xbf && out[4] == 0xbf);
    CHECK(rps_init(&t, 1, mode) == RPS_OK);
    for (i = 1; i <= 16; ++i)
      CHECK(rps_remove(&t, 1, i) == RPS_OK);
    CHECK(rps_insert(&t, 1, 20, 0, 'z') == (mode ? RPS_FULL : RPS_OK));
    CHECK(rps_remove(&t, 1, 17) == RPS_FULL);
  }
  puts("SEQUENCE_TREE early-delete unicode queue-versus-placeholder");
  return 0;
}
static int snapshot(void) {
  struct rps_tree a, b, c, out, saved;
  unsigned char wire[368], bad[368], render[64];
  unsigned i;
  size_t n;
  CHECK(rps_init(&a, 1, 0) == RPS_OK);
  CHECK(rps_init(&b, 1, 1) == RPS_OK);
  CHECK(rps_init(&c, 1, 0) == RPS_OK);
  CHECK(rps_insert(&a, 1, 1, 0, 'a') == RPS_OK);
  CHECK(rps_insert(&a, 1, 2, 0, 'b') == RPS_OK);
  CHECK(rps_insert(&a, 1, 3, 1, 0x20ac) == RPS_OK);
  CHECK(rps_remove(&a, 1, 1) == RPS_OK);
  CHECK(rps_remove(&b, 1, 1) == RPS_OK);
  CHECK(rps_insert(&b, 1, 3, 1, 0x20ac) == RPS_OK);
  CHECK(rps_insert(&b, 1, 2, 0, 'b') == RPS_OK);
  CHECK(rps_insert(&b, 1, 1, 0, 'a') == RPS_OK);
  c = a;
  CHECK(rps_snapshot(&a, wire) == RPS_OK);
  CHECK(rps_restore(wire, &out) == RPS_OK);
  saved = out;
  for (i = 0; i < 368 * 8; ++i) {
    memcpy(bad, wire, 368);
    bad[i / 8] ^= (unsigned char)(1u << (i % 8));
    CHECK(rps_restore(bad, &out) == RPS_INVALID &&
          !memcmp(&out, &saved, sizeof out));
  }
  memcpy(bad, wire, 368);
  bad[15] = 1;
  (void)rph_hash(bad, 336, bad + 336);
  CHECK(rps_restore(bad, &out) == RPS_INVALID);
  saved = a;
  CHECK(rps_compact(&a, &b, &c, 1) == RPS_BUSY &&
        !memcmp(&a, &saved, sizeof a));
  CHECK(rps_compact(&a, &b, &c, 0) == RPS_OK && a.epoch == 2 && a.count == 2 &&
        b.mode == 1);
  CHECK(rps_render(&a, render, 64, &n) == RPS_OK && n == 4 &&
        render[0] == 'b' && render[1] == 0xe2);
  CHECK(rps_remove(&a, 1, 1) == RPS_STALE);
  CHECK(rps_remove(&a, 2, 1) == RPS_OK);
  CHECK(rps_compact(&a, &b, &c, 0) == RPS_GAP);
  a = b = c = saved;
  a.epoch = b.epoch = c.epoch = UINT64_MAX;
  CHECK(rps_compact(&a, &b, &c, 0) == RPS_RANGE);
  CHECK(rps_init(&a, 1, 0) == RPS_OK);
  b = c = a;
  CHECK(rps_compact(&a, &b, &c, 0) == RPS_OK && !a.count && a.epoch == 2);
  CHECK(rps_render(&a, NULL, 0, &n) == RPS_OK && n == 0);
  CHECK(rps_insert(&a, 2, 2, 1, 'x') == RPS_OK);
  b = c = a;
  CHECK(rps_compact(&a, &b, &c, 0) == RPS_GAP);
  puts("SEQUENCE_SNAPSHOT bits2944 quiescent-fence retained-descendants");
  return 0;
}
static struct rps_server server;
static struct rps_client clients[3];
static struct rps_request requests[3];
static struct rps_commit commits[16];
static int setup(void) {
  unsigned i;
  CHECK(rps_server_init(&server, 1) == RPS_OK);
  for (i = 0; i < 3; ++i)
    CHECK(rps_client_init(&clients[i], 1, i) == RPS_OK);
  return 0;
}
static int drain(unsigned from, unsigned to) {
  unsigned i, j;
  for (i = from; i <= to; ++i)
    for (j = 0; j < 3; ++j)
      CHECK(rps_deliver(&clients[j], &commits[i - 1]) == RPS_OK);
  return 0;
}
static int context(void) {
  struct rps_request bad;
  struct rps_commit out, saved;
  struct rps_server original;
  unsigned i;
  CHECK(setup() == 0);
  for (i = 0; i < 3; ++i)
    CHECK(rps_prepare(&clients[i], 1, 0, 'a' + i, &requests[i]) == RPS_OK);
  bad = requests[2];
  bad.vector[0] = 1;
  CHECK(rps_submit(&server, &bad, &out) == RPS_GAP);
  CHECK(rps_submit(&server, &requests[2], &commits[0]) == RPS_OK);
  CHECK(rps_submit(&server, &requests[0], &commits[1]) == RPS_OK);
  CHECK(rps_submit(&server, &requests[1], &commits[2]) == RPS_OK);
  CHECK(server.lengths[3] == 3 && server.documents[3][0] == 'a' &&
        server.documents[3][1] == 'b' && server.documents[3][2] == 'c');
  CHECK(drain(1, 3) == 0);
  CHECK(rps_prepare(&clients[0], 2, 1, 0, &requests[0]) == RPS_OK);
  CHECK(rps_prepare(&clients[1], 2, 1, 0, &requests[1]) == RPS_OK);
  CHECK(rps_submit(&server, &requests[0], &commits[3]) == RPS_OK);
  CHECK(rps_submit(&server, &requests[1], &commits[4]) == RPS_OK &&
        commits[4].op.kind == 0);
  CHECK(server.lengths[5] == 2 && server.documents[5][0] == 'a' &&
        server.documents[5][1] == 'c');
  CHECK(drain(4, 5) == 0);
  CHECK(rps_submit(&server, &requests[0], &out) == RPS_OK && out.revision == 4);
  bad = requests[0];
  bad.op.position = 0;
  CHECK(rps_submit(&server, &bad, &out) == RPS_CONFLICT);
  for (i = 6; i <= 16; ++i) {
    CHECK(rps_prepare(&clients[0], 1, 0, 'x', &requests[0]) == RPS_OK);
    CHECK(rps_submit(&server, &requests[0], &commits[i - 1]) == RPS_OK);
    CHECK(drain(i, i) == 0);
  }
  CHECK(rps_prepare(&clients[0], 1, 0, 'z', &requests[0]) == RPS_OK);
  original = server;
  memset(&out, 0xa5, sizeof out);
  memcpy(&saved, &out, sizeof out);
  CHECK(rps_submit(&server, &requests[0], &out) == RPS_FULL &&
        !memcmp(&server, &original, sizeof server) &&
        !memcmp(&out, &saved, sizeof out));
  /* A valid pairwise transform applied to an operation in the wrong prefix
     context moves x to the wrong side of b in this three-operation history. */
  CHECK(setup() == 0);
  CHECK(rps_prepare(&clients[0], 1, 0, 'a', &requests[0]) == RPS_OK);
  CHECK(rps_prepare(&clients[1], 1, 0, 'b', &requests[1]) == RPS_OK);
  CHECK(rps_submit(&server, &requests[0], &commits[0]) == RPS_OK);
  CHECK(rps_deliver(&clients[0], &commits[0]) == RPS_OK);
  CHECK(rps_prepare(&clients[0], 1, 1, 'x', &requests[0]) == RPS_OK);
  CHECK(rps_submit(&server, &requests[1], &commits[1]) == RPS_OK);
  CHECK(rps_submit(&server, &requests[0], &commits[2]) == RPS_OK &&
        commits[2].op.position == 1 && server.documents[3][1] == 'x');
  CHECK(rp_ot_insert(1, 0, requests[1].op.position, 1, &i) == RP_OK && i == 2);
  puts("OT_COUNTEREXAMPLE retained-context=axb blind-original-context=abx");
  puts("OT_CONTEXT exact-prefix vectors retained-transforms duplicate-delete "
       "log-full");
  return 0;
}
static int delivery(void) {
  struct rps_commit bad;
  struct rps_client saved;
  unsigned i;
  CHECK(setup() == 0);
  for (i = 0; i < 3; ++i)
    CHECK(rps_prepare(&clients[i], 1, 0, 'a' + i, &requests[i]) == RPS_OK);
  for (i = 0; i < 3; ++i)
    CHECK(rps_submit(&server, &requests[i], &commits[i]) == RPS_OK);
  saved = clients[2];
  CHECK(rps_deliver(&clients[2], &commits[1]) == RPS_GAP &&
        !memcmp(&saved, &clients[2], sizeof saved));
  CHECK(rps_deliver(&clients[2], &commits[0]) == RPS_OK && clients[2].pending);
  CHECK(rps_deliver(&clients[2], &commits[0]) == RPS_OK);
  bad = commits[0];
  bad.op.scalar = 'z';
  CHECK(rps_deliver(&clients[2], &bad) == RPS_CONFLICT);
  CHECK(rps_deliver(&clients[2], &commits[1]) == RPS_OK && clients[2].pending);
  CHECK(rps_deliver(&clients[2], &commits[2]) == RPS_OK &&
        !clients[2].pending && clients[2].length == 3);
  bad = commits[2];
  bad.epoch = 2;
  CHECK(rps_deliver(&clients[2], &bad) == RPS_STALE);
  puts("OT_DELIVERY gap duplicate-conflict pending-through-other-commit");
  return 0;
}
static int tree_oracle(void) {
  struct rps_tree t;
  char op;
  unsigned a, b, c;
  unsigned char wire[368], bytes[64];
  size_t n, i;
  int r;
  while (scanf(" %c %u %u %u", &op, &a, &b, &c) == 4) {
    if (op == 'I')
      r = rps_init(&t, 1, a);
    else if (op == 'A')
      r = rps_insert(&t, 1, a, b, c);
    else if (op == 'D')
      r = rps_remove(&t, 1, a);
    else
      return 2;
    CHECK(rps_snapshot(&t, wire) == RPS_OK);
    CHECK(rps_render(&t, bytes, 64, &n) == RPS_OK);
    printf("%d ", r);
    for (i = 0; i < 368; ++i)
      printf("%02x", wire[i]);
    putchar(' ');
    for (i = 0; i < n; ++i)
      printf("%02x", bytes[i]);
    puts("");
  }
  return 0;
}
static int ot_oracle(void) {
  char op;
  unsigned a, b, c, d, i, j;
  int r;
  while (scanf(" %c %u %u %u %u", &op, &a, &b, &c, &d) == 5) {
    CHECK(a < 3);
    if (op == 'I') {
      CHECK(setup() == 0);
      r = RPS_OK;
    } else if (op == 'P')
      r = rps_prepare(&clients[a], b, c, d, &requests[a]);
    else if (op == 'S') {
      struct rps_commit out;
      r = rps_submit(&server, &requests[a], &out);
      if (!r)
        commits[out.revision - 1] = out;
    } else if (op == 'D') {
      CHECK(b > 0 && b <= server.count);
      r = rps_deliver(&clients[a], &commits[b - 1]);
    } else
      return 2;
    printf("%d %u %u", r, server.count, server.lengths[server.count]);
    for (i = 0; i < server.lengths[server.count]; ++i)
      printf(" %u", server.documents[server.count][i]);
    for (i = 0; i < 3; ++i) {
      struct rps_client *x = &clients[i];
      printf(" %u %u %u", x->revision, x->pending, x->length);
      for (j = 0; j < 3; ++j)
        printf(" %u", x->vector[j]);
      for (j = 0; j < x->length; ++j)
        printf(" %u", x->document[j]);
    }
    puts("");
  }
  return 0;
}
int main(int argc, char **argv) {
  if (argc != 2)
    return 2;
  if (!strcmp(argv[1], "seq_tree"))
    return tree();
  if (!strcmp(argv[1], "seq_snapshot"))
    return snapshot();
  if (!strcmp(argv[1], "ot_context"))
    return context();
  if (!strcmp(argv[1], "ot_delivery"))
    return delivery();
  if (!strcmp(argv[1], "tree_oracle"))
    return tree_oracle();
  if (!strcmp(argv[1], "ot_oracle"))
    return ot_oracle();
  if (!strcmp(argv[1], "metrics")) {
    printf("{\"tree_size\":%zu,\"server_size\":%zu,\"client_size\":%zu,"
           "\"snapshot_bytes\":368,\"node_capacity\":16,\"log_capacity\":16}\n",
           sizeof(struct rps_tree), sizeof(struct rps_server), sizeof(struct rps_client));
    return 0;
  }
  return 2;
}
