#include "conversion.h"
#include "scanner.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "SCANNER-CHECK: %s line%d\n", #x, __LINE__);             \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static FILE *input(const char *s) {
  FILE *f = tmpfile();
  if (!f)
    return NULL;
  size_t n = strlen(s);
  if (fwrite(s, 1, n, f) != n || fseek(f, 0, SEEK_SET)) {
    fclose(f);
    return NULL;
  }
  return f;
}
int main(void) {
  char token[4] = {'X', 'X', 'X', 'X'};
  int consumed = -1;
  CHECK(sscanf("ABC!", "%3s%n", token, &consumed) == 1 && consumed == 3 &&
        !strcmp(token, "ABC"));
  CHECK(sscanf(" ", "%3s", token) == EOF && !strcmp(token, "ABC"));
  const char *valid = "count=12;name=AB";
  struct ls_record out = {0};
  CHECK(ls_record_parse((const unsigned char *)valid, strlen(valid), 0, &out) ==
            LS_OK &&
        out.count == 12 && out.length == 2 && !strcmp((char *)out.name, "AB"));
  const char *bad[] = {
      "count=12name=AB",         "count=12;name=",    "count=12;name=AB!",
      "count=12;name=AB;",       "count=12;name=A B", "count=-1;name=AB",
      "count=2147483648;name=AB"};
  for (size_t i = 0; i < sizeof bad / sizeof *bad; ++i) {
    memset(&out, 0x55, sizeof out);
    unsigned char old[sizeof out];
    memcpy(old, &out, sizeof out);
    CHECK(ls_record_parse((const unsigned char *)bad[i], strlen(bad[i]), 0,
                          &out) != LS_OK &&
          !memcmp(old, &out, sizeof out));
  }
  const char *escaped = "count=12;name=A\\;B\\\\C";
  CHECK(ls_record_parse((const unsigned char *)escaped, strlen(escaped), 1,
                        &out) == LS_OK &&
        !strcmp((char *)out.name, "A;B\\C"));
  CHECK(ls_record_parse((const unsigned char *)escaped, strlen(escaped), 0,
                        &out) == LS_SYNTAX);
  /* scanf may publish count before a later field fails. Only representable12.
   */
  int count = -1;
  char name[33] = "OLD";
  consumed = -1;
  CHECK(sscanf("count=12name=AB", "count=%d;name=%32[A-Za-z0-9_]%n", &count,
               name, &consumed) == 1 &&
        count == 12 && !strcmp(name, "OLD") && consumed == -1);
  count = -1;
  consumed = -1;
  CHECK(sscanf("count=12;name=AB!", "count=%d;name=%32[A-Za-z0-9_]%n", &count,
               name, &consumed) == 2 &&
        count == 12 && consumed == 16);
  const char *texts[] = {"12x", "x", "", "12"};
  int assignments[] = {1, 0, EOF, 1}, suffix[] = {'x', 'x', EOF, EOF};
  for (size_t i = 0; i < 4; ++i) {
    FILE *f = input(texts[i]);
    CHECK(f);
    CHECK(!feof(f));
    count = 77;
    int a = fscanf(f, "%d", &count);
    CHECK(a == assignments[i]);
    CHECK((a == 1 && count == 12) || (a != 1 && count == 77));
    int after = fgetc(f);
    CHECK(after == suffix[i]);
    CHECK(!ferror(f));
    printf("trace input=%zu assigned=%d unread=%d eof_after=%d\n", i, a, after,
           feof(f) != 0);
    CHECK(fclose(f) == 0);
  }
  FILE *f = input("123456789\n12x\n42\n\n7");
  CHECK(f);
  unsigned char line[129];
  size_t length = 99;
  memset(line, 0x55, sizeof line);
  CHECK(ls_line(f, line, 4, &length) == LS_LONG && length == 99 &&
        line[0] == 0x55);
  CHECK(ls_line(f, line, sizeof line, &length) == LS_OK && length == 3 &&
        !memcmp(line, "12x", 3));
  uintmax_t value = 77;
  CHECK(lc_unsigned(line, length, 0, 100, &value) == LC_SYNTAX && value == 77);
  CHECK(ls_line(f, line, sizeof line, &length) == LS_OK && length == 2 &&
        lc_unsigned(line, length, 0, 100, &value) == LC_OK && value == 42);
  CHECK(ls_line(f, line, sizeof line, &length) == LS_OK && length == 0);
  CHECK(ls_line(f, line, sizeof line, &length) == LS_OK && length == 1 &&
        line[0] == '7');
  CHECK(ls_line(f, line, sizeof line, &length) == LS_END);
  CHECK(fclose(f) == 0);
  unsigned char longtoken[128];
  memset(longtoken, '0', 127);
  longtoken[127] = '9';
  CHECK(lc_unsigned(longtoken, sizeof longtoken, 0, 10, &value) == LC_OK &&
        value == 9);
  memset(longtoken, '9', sizeof longtoken);
  CHECK(lc_unsigned(longtoken, sizeof longtoken, 0, UINTMAX_MAX, &value) ==
            LC_RANGE &&
        value == 9);
  puts("SCANNER-PASS: bounded assignment, transactional records, escape state "
       "and consuming line recovery");
  return 0;
}
