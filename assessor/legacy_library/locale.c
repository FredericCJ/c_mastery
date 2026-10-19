#define _XOPEN_SOURCE 700
#include "unicode.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <locale.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "LOCALE-CHECK:%d:%s\n", __LINE__, #x);                   \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int ascii_space(unsigned char c) {
  return c == 9 || c == 10 || c == 11 || c == 12 || c == 13 || c == 32;
}
struct worker {
  locale_t locale;
  int utf8, failed;
};
static void *work(void *v) {
  struct worker *w = v;
  locale_t previous = uselocale(w->locale);
  if (!previous) {
    w->failed = 1;
    return NULL;
  }
  for (unsigned k = 0; k < 1000; ++k) {
    mbstate_t state = {0};
    wchar_t value = 99;
    errno = 0;
    size_t n = mbrtowc(&value, "\xe2\x82\xac", 3, &state);
    if (w->utf8 ? (n != 3 || value != 0x20ac)
                : (n != (size_t)-1 || errno != EILSEQ))
      w->failed = 1;
    for (unsigned i = 0; i <= UCHAR_MAX; ++i)
      if (!!isspace_l((int)i, w->locale) != ascii_space((unsigned char)i))
        w->failed = 1;
  }
  if (!uselocale(previous))
    w->failed = 1;
  return NULL;
}
int main(void) {
  CHECK(CHAR_BIT == 8 && sizeof(wchar_t) == 4 && setlocale(LC_ALL, "C"));
  for (unsigned i = 0; i <= UCHAR_MAX; ++i) {
    char narrowed = (char)(unsigned char)i;
    CHECK(!!isspace((unsigned char)narrowed) == ascii_space((unsigned char)i));
  }
  FILE *f = tmpfile();
  CHECK(f);
  for (unsigned i = 0; i <= UCHAR_MAX; ++i)
    CHECK(fputc((int)i, f) != EOF);
  rewind(f);
  for (unsigned i = 0; i <= UCHAR_MAX; ++i) {
    int x = fgetc(f);
    CHECK(x != EOF && x == (int)i);
    CHECK(!!isspace(x) == ascii_space((unsigned char)i));
  }
  CHECK(fgetc(f) == EOF && feof(f) && !ferror(f));
  CHECK(fclose(f) == 0);
  /* ISO conversion to signed char is implementation-defined out of range;
     assert the selected two's-complement Linux compiler behavior, not
     portability. */
  CHECK(CHAR_MIN < 0 && (int)(signed char)0xff == EOF && EOF == -1);
  printf("EOF-COLLISION observed signed-byte=%d eof=%d plain-char-min=%d\n",
         (int)(signed char)0xff, EOF, CHAR_MIN);
  CHECK(setlocale(LC_CTYPE, "C.utf8"));
  printf("LOCALE %s wchar=%zu MB_CUR_MAX=%zu\n", setlocale(LC_CTYPE, NULL),
         sizeof(wchar_t), (size_t)MB_CUR_MAX);
  mbstate_t a = {0}, b = {0};
  wchar_t value = 99;
  CHECK(mbrtowc(&value, "\xe2", 1, &a) == (size_t)-2 && value == 99 &&
        !mbsinit(&a));
  CHECK(mbrtowc(&value, "\x82\xac", 2, &a) == 2 && value == 0x20ac &&
        mbsinit(&a));
  const char face[] = "\xf0\x9f\x98\x80";
  for (size_t cut = 1; cut < 4; ++cut) {
    a = (mbstate_t){0};
    b = (mbstate_t){0};
    CHECK(mbrtowc(&value, face, cut, &a) == (size_t)-2);
    CHECK(mbrtowc(&value, "\xe2", 1, &b) == (size_t)-2);
    CHECK(mbrtowc(&value, face + cut, 4 - cut, &a) == 4 - cut &&
          value == 0x1f600);
    CHECK(mbrtowc(&value, "\x82\xac", 2, &b) == 2 && value == 0x20ac);
  }
  const char *bad[] = {"\xc0\xaf", "\xed\xa0\x80"};
  for (unsigned i = 0; i < 2; ++i) {
    a = (mbstate_t){0};
    value = 99;
    errno = 0;
    CHECK(mbrtowc(&value, bad[i], strlen(bad[i]), &a) == (size_t)-1 &&
          errno == EILSEQ &&
          value == 99); /* Never inspect/use unspecified state after error. */
    a = (mbstate_t){0};
    CHECK(mbrtowc(&value, "A", 1, &a) == 1 && value == 'A');
  }
  a = (mbstate_t){0};
  CHECK(mbrtowc(&value, "", 1, &a) == 0 && value == 0 && mbsinit(&a));
  const wchar_t values[] = {0x41, 0x20ac, 0x1f600};
  const char *expected[] = {"A", "\xe2\x82\xac", face};
  for (unsigned i = 0; i < 3; ++i) {
    char bytes[MB_LEN_MAX];
    a = (mbstate_t){0};
    size_t n = wcrtomb(bytes, values[i], &a);
    CHECK(n == strlen(expected[i]) && !memcmp(bytes, expected[i], n));
  }
  const unsigned char combining[] = {0x65, 0xcc, 0x81};
  struct lu_state s = {0};
  struct lu_result r = {0};
  uint32_t points[3] = {0};
  CHECK(lu_decode(&s, combining, 3, points, 3, LU_STRICT, &r) == LU_OK &&
        r.produced == 2 && points[0] == 0x65 &&
        points[1] == 0x301); /* Grapheme/display claims need their separate
                                version/rendering contract. */
  CHECK(setlocale(LC_ALL, "C"));
  locale_t c = newlocale(LC_ALL_MASK, "C", (locale_t)0),
           u = newlocale(LC_ALL_MASK, "C.utf8", (locale_t)0);
  CHECK(c && u);
  struct worker workers[] = {{c, 0, 0}, {u, 1, 0}};
  pthread_t threads[2];
  CHECK(!pthread_create(&threads[0], NULL, work, &workers[0]));
  CHECK(!pthread_create(&threads[1], NULL, work, &workers[1]));
  CHECK(!pthread_join(threads[0], NULL) && !pthread_join(threads[1], NULL));
  CHECK(!workers[0].failed && !workers[1].failed &&
        !strcmp(setlocale(LC_ALL, NULL), "C"));
  freelocale(c);
  freelocale(u);
  puts("LOCALE-PASS:256-bytes:eof:chunks:reset:two-thread-policies");
  return 0;
}
