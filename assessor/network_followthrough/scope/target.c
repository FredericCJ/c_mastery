#include "pair.h"
#include "scope.h"
#include <stdint.h>
__attribute__((section(".dma"), aligned(32),
               used)) static unsigned char dma_buffers[6][2048];
__attribute__((section(".dma"), aligned(32),
               used)) static uint32_t dma_descriptors[6][4];
static NwTx connections[2];
void nw_print(const char *s) {
  register unsigned r0 __asm__("r0") = 4;
  register const char *r1 __asm__("r1") = s;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}
int puts(const char *s) {
  nw_print(s);
  nw_print("\n");
  return 0;
}
__attribute__((noreturn)) static void finish(unsigned code) {
  unsigned args[2] = {0x20026, code};
  register unsigned r0 __asm__("r0") = 0x20;
  register unsigned *r1 __asm__("r1") = args;
  __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
  for (;;) {
  }
}
void xf_fault(void) {
  puts("NW_TARGET_FAULT");
  finish(2);
}
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      puts("NW_TARGET_CHECK " #x);                                             \
      finish(1);                                                               \
    }                                                                          \
  } while (0)
int nw_integration(void);
int nw_pair_test(void);
static void number(unsigned value) {
  char text[11];
  unsigned n = 0;
  do {
    text[n++] = (char)('0' + value % 10);
    value /= 10;
  } while (value);
  while (n) {
    char c[2] = {text[--n], 0};
    nw_print(c);
  }
}
void xf_target_main(void) {
  NwLengths lengths;
  CHECK(nw_lengths(3, &lengths) == NW_OK && lengths.with_fcs == 64);
  nw_print("NW_LAYOUT ");
  number(sizeof(NetStack));
  nw_print(" ");
  number(sizeof(NwPair));
  nw_print(" ");
  number(sizeof(NwTx));
  nw_print(" ");
  number(sizeof(NwRx));
  nw_print("\n");
  for (unsigned i = 0; i < 6; ++i) {
    for (unsigned j = 0; j < 2048; ++j)
      dma_buffers[i][j] = 0;
    for (unsigned j = 0; j < 4; ++j)
      dma_descriptors[i][j] = 0;
  }
  CHECK(sizeof dma_buffers + sizeof dma_descriptors == 12384);
  CHECK(((uintptr_t)dma_buffers & 31) == 0 &&
        ((uintptr_t)dma_descriptors & 31) == 0);
  CHECK((uintptr_t)dma_buffers >= 0x20000000 &&
        (uintptr_t)dma_buffers + sizeof dma_buffers <= 0x20008000);
  unsigned char source[600];
  for (unsigned i = 0; i < 600; ++i)
    source[i] = (unsigned char)i;
  for (unsigned k = 0; k < 2; ++k) {
    CHECK(nw_tx_init(&connections[k], 1, k + 40000) == NW_OK);
    CHECK(nw_offer(&connections[k], source, 600) == NW_OK);
  }
  for (unsigned k = 0; k < 2; ++k) {
    unsigned packets = 0;
    NwTx *s = &connections[k];
    while (s->acknowledged < 600) {
      NwSegment segment;
      CHECK(nw_prepare(s, 256, 512, &segment) == NW_OK);
      CHECK(segment.length == (packets < 2 ? 256 : 88));
      CHECK(nw_submit(s, segment.ticket, 1) == NW_OK);
      if (k == 0)
        CHECK(nw_dma_done(s, segment.ticket) == NW_OK && s->flight);
      CHECK(nw_ack(s, segment.ticket, s->sent) == NW_OK);
      if (k == 1)
        CHECK(s->dma && nw_dma_done(s, segment.ticket) == NW_OK);
      ++packets;
    }
    CHECK(packets == 3);
  }
  CHECK(!nw_integration());
  CHECK(!nw_pair_test());
  puts("NW_TARGET_PASS dma_reserved=12384 connections=2 segments_each=3 "
       "physical_dma=unobserved");
  finish(0);
}
