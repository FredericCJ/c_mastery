#include "test.h"
#ifndef AR_STATIC_POLICY
#define AR_STATIC_POLICY 1
#endif
typedef struct {
  unsigned char bytes[8];
  size_t length;
  unsigned fail;
} Sink;
typedef ArResult (*Write)(void *, const unsigned char *, size_t);
static ArResult write_width(void *p, const unsigned char *b, size_t n,
                            size_t width) {
  Sink *s = p;
  if (s->fail)
    return (ArResult){0, AR_FAILED};
  if (n > width)
    n = width;
  for (size_t i = 0; i < n; ++i)
    s->bytes[s->length++] = b[i];
  return (ArResult){(ptrdiff_t)n, AR_OK};
}
static ArResult one(void *p, const unsigned char *b, size_t n) {
  return write_width(p, b, n, 1);
}
static ArResult two(void *p, const unsigned char *b, size_t n) {
  return write_width(p, b, n, 2);
}
__attribute__((noinline)) static ArResult
direct(void *p, const unsigned char *b, size_t n) {
#if AR_STATIC_POLICY == 1
  return one(p, b, n);
#else
  return two(p, b, n);
#endif
}
__attribute__((noinline)) static int pump(Write w, Sink *s) {
  const unsigned char bytes[] = {1, 2, 3};
  size_t offset = 0;
  while (offset < 3) {
    ArResult r = w(s, bytes + offset, 3 - offset);
    if (r.state != AR_OK)
      return AR_FAILED;
    if (r.count <= 0 || (size_t)r.count > 3 - offset)
      return AR_CONTRACT;
    offset += (size_t)r.count;
  }
  return AR_OK;
}
int main(void) {
  Write policies[] = {one, two, direct};
  for (unsigned i = 0; i < 3; ++i) {
    Sink s = {0};
    CHECK(pump(policies[i], &s) == AR_OK && s.length == 3 &&
          !memcmp(s.bytes, (unsigned char[]){1, 2, 3}, 3));
    s = (Sink){.fail = 1};
    CHECK(pump(policies[i], &s) == AR_FAILED && s.length == 0);
  }
  printf("PASS compiletime=%d runtime2policies sizeofFunctionPointer=%zu "
         "sizeofSink=%zu same progress/error contract\n",
         AR_STATIC_POLICY, sizeof(Write), sizeof(Sink));
}
