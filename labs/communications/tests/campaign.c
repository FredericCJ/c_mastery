#include "com_edu7.h"
#include "com_http.h"
#include "com_net.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, #x);                  \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int feed(const uint8_t *p, size_t n) {
  struct http_parser h;
  http_init(&h, "127.0.0.1:8080");
  size_t off = 0;
  while (off < n) {
    size_t used = 0;
    int r = http_feed(&h, p + off, n - off, &used);
    CHECK(used <= n - off);
    off += used;
    if (r == HTTP_DONE || r == HTTP_ERROR)
      break;
    CHECK(used > 0);
  }
  http_eof(&h);
  CHECK(h.request.body_length <= 1024 && h.line_length <= 8192);
  return 0;
}
int main(void) {
  const uint8_t seed[] =
      "POST /echo HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nTransfer-Encoding: "
      "chunked\r\n\r\n3;x=y\r\nabc\r\n0\r\nX-End: yes\r\n\r\n";
  uint8_t changed[sizeof seed];
  for (size_t i = 0; i < sizeof seed - 1; i++)
    for (unsigned bit = 0; bit < 8; bit++) {
      memcpy(changed, seed, sizeof seed);
      changed[i] ^= (uint8_t)(1u << bit);
      CHECK(!feed(changed, sizeof seed - 1));
    }
  uint32_t rng = 0x5eed1234;
  uint8_t bytes[512];
  for (unsigned trial = 0; trial < 4000; trial++) {
    for (size_t i = 0; i < sizeof bytes; i++) {
      rng = rng * 1664525 + 1013904223;
      bytes[i] = (uint8_t)(rng >> 24);
    }
    size_t n = trial % (sizeof bytes + 1);
    CHECK(!feed(bytes, n));
    struct edu_record r;
    edu_record_decode(bytes, n, &r);
    struct edu_datagram d;
    edu_network_decode(bytes, n, &d);
    struct edu_fragment f;
    edu_fragment_decode(bytes, n, &f);
    struct net_stack s;
    struct net_packet out;
    net_init(&s, 0);
    net_receive(&s, bytes, n, 0, &out);
  }
  /* Required positive control keeps this campaign from passing inert stubs. */
  CHECK(edu_crc("123456789", 9) == 0x29b1);
  struct http_parser p;
  http_init(&p, "127.0.0.1:8080");
  size_t used;
  CHECK(http_feed(&p, seed, sizeof seed - 1, &used) == HTTP_DONE &&
        p.request.body_length == 3);
  char huge[8300];
  memset(huge, 'a', sizeof huge);
  memcpy(huge, "GET / HTTP/1.1\r\nX: ", 19);
  http_init(&p, "127.0.0.1:8080");
  CHECK(http_feed(&p, huge, sizeof huge, &used) == HTTP_ERROR &&
        p.request.status == 431 && used == 8193);
  const char *repeat =
      "POST /echo HTTP/1.1\r\nHost: 127.0.0.1:8080\r\nTransfer-Encoding: gzip, "
      "GZIP, chunked\r\n\r\n";
  http_init(&p, "127.0.0.1:8080");
  CHECK(http_feed(&p, repeat, strlen(repeat), &used) == HTTP_ERROR &&
        p.request.status == 400);
  puts("PASS deterministic malformed-byte campaign; finite seed and limits, "
       "not exhaustive fuzzing");
  return 0;
}
