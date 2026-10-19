/* Changed-condition consumer: actual bounded NiReassembly, independent fragments. */
#include "network_followthrough/ip/specimens.h"
#include <string.h>
int eh_reassembly(void) {
  NiConfig c = {0xc0000202, 0xc0000201};
  NiReassembly s;
  NiPacket out;
  unsigned char p[44];
  unsigned index = 99;
  CHECK(ni_init(&s, c, 1, NI_IDENTICAL, 0, 60000) == NI_OK);
  ni_test_header(p, 8, 17, 16, 0, 6);
  for (unsigned i = 0; i < 8; ++i) p[20+i] = (unsigned char)(16+i);
  CHECK(ni_receive(&s, p, 28, 0, &index) == NI_WAIT);
  ni_test_header(p, 8, 17, 0, 1, 6);
  for (unsigned i = 0; i < 8; ++i) p[20+i] = (unsigned char)i;
  CHECK(ni_receive(&s, p, 28, 1, &index) == NI_WAIT);
  out.length = 999;
  CHECK(ni_take(&s, index, &out) == NI_WAIT && out.length == 999);
  ni_test_header(p, 8, 17, 8, 1, 6);
  for (unsigned i = 0; i < 8; ++i) p[20+i] = (unsigned char)(8+i);
  CHECK(ni_receive(&s, p, 28, 2, &index) == NI_OK);
  CHECK(ni_take(&s, index, &out) == NI_OK && out.length == 44);
  for (unsigned i = 0; i < 24; ++i) CHECK(out.bytes[20+i] == i);
  CHECK(!ni_test_sum(out.bytes, 20) && !out.bytes[6] && !out.bytes[7]);
  CHECK(ni_receive(&s, p, 28, 3, &index) == NI_CONFLICT);
  CHECK(ni_tick(&s, 60000) == NI_OK && s.slot[0].state == NI_FREE);
  /* Fresh identity. Identical overlap keeps the original deadline. Changed
   * overlap poisons instead of selecting arrival-dependent application bytes. */
  ni_test_header(p, 16, 18, 0, 1, 6);
  for (unsigned i = 0; i < 16; ++i) p[20+i] = (unsigned char)i;
  CHECK(ni_receive(&s, p, 36, 60000, &index) == NI_WAIT);
  CHECK(ni_receive(&s, p, 36, 60001, &index) == NI_WAIT && s.slot[0].expiry == 120000);
  p[20] ^= 1;
  CHECK(ni_receive(&s, p, 36, 60002, &index) == NI_CONFLICT);
  CHECK(ni_tick(&s, 119999) == NI_OK && s.slot[0].state == NI_POISON);
  CHECK(ni_tick(&s, 120000) == NI_OK && s.slot[0].state == NI_FREE);
  puts("EH_REASSEMBLY_PASS holes=8 output=44 tombstone=60000 immutable_deadline=120000 conflicting_overlap_poison");
  return 0;
}
