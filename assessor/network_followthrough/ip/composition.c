#include "com_net.h"
#include "specimens.h"
static struct net_stack stack;
static struct ni_reassembly reassembly;
static const struct ni_config config = {0xc0000202u, 0xc0000201u};
static void frame(struct net_packet *p, const unsigned char *ip, size_t n) {
  const unsigned char h[14] = {2, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 1, 8, 0};
  for (unsigned i = 0; i < 14; i++)
    p->bytes[i] = h[i];
  for (size_t i = 0; i < n; i++)
    p->bytes[14 + i] = ip[i];
  p->length = n + 14;
  if (p->length < 60)
    p->length = 60;
  for (size_t i = n + 14; i < p->length; i++)
    p->bytes[i] = 0;
}
int main(void) {
  unsigned char request[100], fragment[100];
  struct net_packet input, actual;
  struct ni_packet expected, whole;
  for (unsigned n = 0; n <= 64; n++) {
    ni_test_echo(request, n);
    frame(&input, request, n + 28);
    net_init(&stack, 1000);
    const unsigned char arp[60] = {
        255, 255, 255, 255, 255, 255, 2, 0, 0, 0, 0,   1, 8, 6,
        0,   1,   8,   0,   6,   4,   0, 1, 2, 0, 0,   0, 0, 1,
        192, 0,   2,   1,   0,   0,   0, 0, 0, 0, 192, 0, 2, 2};
    CHECK(net_receive(&stack, arp, sizeof arp, 0, &actual) == 1 &&
          stack.peer_known);
    CHECK(net_receive(&stack, input.bytes, input.length, 0, &actual) >= 0);
    CHECK(ni_echo(&config, input.bytes + 14, input.length - 14, &expected) ==
          NI_OK);
    CHECK(actual.length == ((n + 42 < 60) ? 60 : n + 42));
    for (size_t i = 0; i < expected.length; i++)
      CHECK(actual.bytes[14 + i] == expected.bytes[i]);
    for (size_t i = 14 + expected.length; i < actual.length; i++)
      CHECK(actual.bytes[i] == 0);
    /* Actual existing parser remains strict; our explicit reassembly feeds it
     * only after atomic delivery of the complete payload. */
    if (n == 0)
      continue;
    CHECK(ni_init(&reassembly, config, 2, NI_STRICT, 0, 60000) == NI_OK);
    ni_test_header(fragment, n, 55, 8, 0, 1);
    for (unsigned j = 0; j < n; j++)
      fragment[20 + j] = request[28 + j];
    unsigned slot = 99;
    CHECK(ni_receive(&reassembly, fragment, n + 20, 1, &slot) == NI_WAIT);
    ni_test_header(fragment, 8, 55, 0, 1, 1);
    for (unsigned j = 0; j < 8; j++)
      fragment[20 + j] = request[20 + j];
    frame(&input, fragment, 28);
    actual.length = 999;
    CHECK(net_receive(&stack, input.bytes, input.length, 2, &actual) >= 0 &&
          actual.length == 0);
    CHECK(ni_receive(&reassembly, fragment, 28, 2, &slot) == NI_OK);
    CHECK(ni_take(&reassembly, slot, &whole) == NI_OK);
    frame(&input, whole.bytes, whole.length);
    CHECK(net_receive(&stack, input.bytes, input.length, 3, &actual) >= 0 &&
          actual.length);
    for (size_t i = 0; i < expected.length; i++)
      CHECK(actual.bytes[14 + i] == expected.bytes[i]);
  }
  /* Feed the real streaming checksum a TCP pseudoheader in all two-split
   * arrangements. Expected value comes from the actual contiguous provider
   * AND the independent mathematical builder; both must agree. */
  const unsigned char pseudo[12] = {192, 0, 2, 1, 192, 0, 2, 2, 0, 6, 0, 5};
  unsigned char joined[17];
  for (unsigned j = 0; j < 12; j++)
    joined[j] = pseudo[j];
  for (unsigned j = 0; j < 5; j++)
    joined[12 + j] = (unsigned char)(0x61 + j);
  for (unsigned split = 0; split <= 17; split++) {
    struct s_checksum sum = {0};
    uint16_t got = 0;
    CHECK(!s_checksum_add(&sum, joined, split));
    CHECK(!s_checksum_add(&sum, joined + split, 17 - split));
    CHECK(!s_checksum_finish(&sum, &got));
    CHECK(got == net_checksum(joined, 17, 0) && got == ni_test_sum(joined, 17));
  }
  struct ni_packet errors[2];
  unsigned count = 99, slot;
  CHECK(ni_init(&reassembly, config, 2, NI_STRICT, 0, 60000) == NI_OK);
  ni_test_header(fragment, 8, 8, 0, 1, 6);
  for (unsigned j = 0; j < 8; j++)
    fragment[20 + j] = (unsigned char)(40 + j);
  CHECK(ni_receive(&reassembly, fragment, 28, 0, &slot) == NI_WAIT);
  CHECK(ni_expire(&reassembly, 59999, errors, &count) == NI_OK && count == 0);
  CHECK(ni_expire(&reassembly, 60000, errors, &count) == NI_OK && count == 1);
  CHECK(errors[0].length == 56 && errors[0].bytes[20] == 11 &&
        errors[0].bytes[21] == 1);
  for (unsigned j = 0; j < 28; j++)
    CHECK(errors[0].bytes[28 + j] == fragment[j]);
  CHECK(ni_test_sum(errors[0].bytes, 20) == 0 &&
        ni_test_sum(errors[0].bytes + 20, 36) == 0);
  CHECK(ni_expire(&reassembly, 60000, errors, &count) == NI_OK && count == 0);
  CHECK(ni_init(&reassembly, config, 2, NI_STRICT, 0, 60000) == NI_OK);
  ni_test_header(fragment, 8, 8, 8, 1, 6);
  CHECK(ni_receive(&reassembly, fragment, 28, 0, &slot) == NI_WAIT);
  ni_test_header(fragment, 8, 9, 0, 1, 1);
  fragment[20] = 3;
  CHECK(ni_receive(&reassembly, fragment, 28, 0, &slot) == NI_WAIT);
  CHECK(ni_expire(&reassembly, 60000, errors, &count) == NI_OK && count == 0);
  count = 99;
  CHECK(ni_expire(&reassembly, 59999, errors, &count) == NI_INVALID &&
        count == 99);
  puts("NI_COMPOSITION_PASS actual_net_checksum_Echo_reassembly_timeout");
  return 0;
}
