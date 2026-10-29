#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "FT_CHECK:%s:%d:%s\n", __FILE__, __LINE__, #x);          \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
static void cases(void) {
  const char *normal[] = {"2", "-5", "9"}, *bad[] = {"32767", "1", "-1"};
  struct ft_result out = {91, 92};
  struct ft_errors e;
  CHECK(ft_tokens(normal, 3, 0, &out, &e) == FT_OK && out.count == 3 &&
        out.sum == 6 && e.total == 0);
  out = (struct ft_result){91, 92};
  CHECK(ft_tokens(bad, 3, 0, &out, &e) == FT_PREFIX && out.count == 91 &&
        out.sum == 92);
  CHECK(e.total == 1 && e.items[0].token == 3 &&
        e.items[0].status == FT_PREFIX);
  CHECK(ft_tokens(NULL, 0, 0, &out, &e) == FT_EMPTY && out.count == 0 &&
        out.sum == 0);
  const char *spelling[] = {"+", "12x", "", " 1", "1 ", "++1", "--2"};
  for (size_t i = 0; i < sizeof spelling / sizeof spelling[0]; i++)
    CHECK(ft_tokens(&spelling[i], 1, 0, &out, &e) == FT_MALFORMED);
  const char *range[] = {"32768", "-32769",
                         "9999999999999999999999999999999999999"};
  for (size_t i = 0; i < 3; i++)
    CHECK(ft_tokens(&range[i], 1, 0, &out, &e) == FT_RANGE);
  const char *limits[] = {"-32768", "32767", "1", "+00000"};
  CHECK(ft_tokens(limits, 4, 0, &out, &e) == FT_OK && out.sum == 0);
  const char *too_many[] = {"0", "0", "0", "0", "0"};
  out = (struct ft_result){91, 92};
  CHECK(ft_tokens(too_many, 5, 0, &out, &e) == FT_CAPACITY_ERROR &&
        out.count == 91 && out.sum == 92);
  const char *errors[] = {"bad", "99999", "+", "1", "2", "--3",
                          "0",   "0",     "0", "0", "0"};
  CHECK(ft_tokens(errors, 11, 1, &out, &e) == FT_MALFORMED && out.count == 91 &&
        out.sum == 92 && e.total == 10 && e.stored == 8);
  CHECK(e.items[0].token == 0 && e.items[1].status == FT_RANGE &&
        e.items[3].token == 4 && e.items[3].status == FT_CAPACITY_ERROR);
  puts("FT_TOKEN_PASS grammar ranges capacity4 atomic_prefix "
       "collected10_stored8");
}
static void streams(void) {
  const unsigned char bytes[] = "2 -5 9";
  for (size_t split = 0; split <= sizeof bytes - 1; split++) {
    struct ft_stream s;
    struct ft_result r = {91, 92};
    ft_stream_init(&s);
    CHECK(ft_stream_feed(&s, bytes, split) == FT_OK);
    CHECK(ft_stream_feed(&s, bytes + split, sizeof bytes - 1 - split) == FT_OK);
    CHECK(ft_stream_finish(&s, &r) == FT_OK && r.count == 3 && r.sum == 6);
    CHECK(ft_stream_finish(&s, &r) == FT_STATE && r.sum == 6);
    CHECK(ft_stream_feed(&s, NULL, 0) == FT_STATE);
  }
  struct ft_stream s;
  struct ft_result r = {91, 92};
  ft_stream_init(&s);
  CHECK(ft_stream_feed(&s, (const unsigned char *)" \t\r\n", 4) == FT_OK);
  CHECK(ft_stream_finish(&s, &r) == FT_EMPTY && r.sum == 0);
  ft_stream_init(&s);
  CHECK(ft_stream_feed(&s, (const unsigned char *)"+", 1) == FT_OK);
  r = (struct ft_result){91, 92};
  CHECK(ft_stream_finish(&s, &r) == FT_MALFORMED && r.sum == 92);
  ft_stream_init(&s);
  const unsigned char embedded[] = {'1', 0, '2'};
  CHECK(ft_stream_feed(&s, embedded, 3) == FT_OK);
  CHECK(ft_stream_finish(&s, &r) == FT_MALFORMED && r.sum == 92);
  ft_stream_init(&s);
  for (unsigned i = 0; i < 1000; i++)
    CHECK(ft_stream_feed(&s, (const unsigned char *)"0", 1) == FT_OK);
  CHECK(ft_stream_feed(&s, (const unsigned char *)"7", 1) == FT_OK);
  CHECK(ft_stream_finish(&s, &r) == FT_OK && r.sum == 7 && r.count == 1);
  const char *failures[] = {"32768", "0 0 0 0 0", "32767 1 -1"};
  const enum ft_status statuses[] = {FT_RANGE, FT_CAPACITY_ERROR, FT_PREFIX};
  for (unsigned j = 0; j < 3; ++j) {
    ft_stream_init(&s);
    r = (struct ft_result){91, 92};
    enum ft_status feed = FT_OK;
    for (const unsigned char *p = (const unsigned char *)failures[j]; *p; ++p) {
      feed = ft_stream_feed(&s, p, 1);
      if (feed != FT_OK)
        break;
    }
    CHECK(feed == FT_OK || feed == statuses[j]);
    CHECK(ft_stream_finish(&s, &r) == statuses[j] && r.count == 91 &&
          r.sum == 92);
  }
  puts("FT_STREAM_PASS all7splits EOF_once embeddedNUL long_leading_zeros");
}
int main(void) {
  cases();
  streams();
  puts("FT_PASS bounded_list_and_changed_input");
  return 0;
}
