#include "bytes.h"
#include "format.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                  \
    do                                                            \
    {                                                             \
        if (!(x))                                                 \
        {                                                         \
            fprintf(stderr, "BYTES-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                             \
        }                                                         \
    } while (0)

static int render(void *c, char *d, size_t n)
{
    (void)c;
    return snprintf(d, n, "%s", "abcdef");
}

int main(void)
{
    size_t cases = 0;
    for (size_t z = 0; z <= 16; ++z)
        for (size_t src = 0; src <= z; ++src)
            for (size_t dst = 0; dst <= z; ++dst)
                for (size_t n = 0; n <= z; ++n)
                {
                    unsigned char b[18], old[18], expected[18];
                    for (unsigned i = 0; i < 18; ++i)
                        b[i] = old[i] = expected[i] = (unsigned char)i;
                    enum lb_status st = lb_move(b, z, dst, src, n);
                    if (n > z - src || n > z - dst)
                        CHECK(st == LB_RANGE && !memcmp(b, old, 18));
                    else
                    {
                        for (size_t i = 0; i < n; ++i)
                            expected[dst + i] = old[src + i];
                        CHECK(st == LB_OK && !memcmp(b, expected, 18));
                    }
                    ++cases;
                }
    CHECK(lb_move(NULL, 0, 0, 0, 0) == LB_OK &&
          lb_move(NULL, 1, 0, 0, 1) == LB_ARGUMENT);
    unsigned char a[5] = {0, 1, 2, 3, 4}, b[5] = {9, 9, 9, 9, 9};
    CHECK(lb_move(a, 5, 1, 0, 4) == LB_OK &&
          !memcmp(a, (unsigned char[]){0, 0, 1, 2, 3}, 5));
    CHECK(lb_copy(b, 5, a, 5, 5) == LB_OK && !memcmp(a, b, 5));
    CHECK(lb_copy(NULL, 0, NULL, 0, 0) == LB_OK &&
          lb_copy(a + 5, 0, b + 5, 0, 0) == LB_OK &&
          lb_copy(NULL, 1, b, 5, 1) == LB_ARGUMENT);
    CHECK(lb_copy(b, 5, a, 5, 6) == LB_RANGE);
    const unsigned char binary[] = {65, 0, 66};
    size_t n = 99;
    CHECK(sizeof binary == 3 && lb_strlen(binary, 3, &n) == LB_OK && n == 1);
    n = 99;
    CHECK(lb_strlen((unsigned char *)"abc", 3, &n) == LB_UNTERMINATED &&
          n == 99);
    CHECK(lb_strlen(NULL, 0, &n) == LB_UNTERMINATED && n == 99);
    char shown[16];
    for (size_t cap = 0; cap <= 10; ++cap)
    {
        memset(shown, '#', sizeof shown);
        n = 99;
        enum lb_status st = lb_display(binary, 3, cap ? shown : NULL, cap, &n);
        if (cap <= 6)
        {
            CHECK(st == LB_FULL && n == 99);
            for (unsigned i = 0; i < 16; ++i)
                CHECK(shown[i] == '#');
        }
        else
            CHECK(st == LB_OK && n == 6 && !strcmp(shown, "A\\x00B") &&
                  shown[7] == '#');
    }
    const uint8_t field[] = {0, 3, 65, 0, 66, 99};
    for (size_t cap = 0; cap <= 10; ++cap)
    {
        struct f_cursor c = {field, sizeof field, 0};
        memset(shown, '#', sizeof shown);
        n = 99;
        enum lb_status st = lb_field_display(&c, cap ? shown : NULL, cap, &n);
        if (cap <= 6)
            CHECK(st == LB_FULL && c.position == 0 && n == 99 &&
                  shown[0] == '#');
        else
            CHECK(st == LB_OK && c.position == 5 && n == 6 &&
                  !strcmp(shown, "A\\x00B"));
    }
    struct f_cursor shortc = {field, 4, 0};
    CHECK(lb_field_display(&shortc, shown, 16, &n) == LB_RANGE &&
          shortc.position == 0);
    const size_t caps[] = {0, 1, 4, 7};
    const size_t written[] = {0, 0, 3, 6};
    for (unsigned i = 0; i < 4; ++i)
    {
        struct lf_result r = {99, 99, 0, 0};
        memset(shown, '#', 16);
        enum lf_status st =
            lf_prefix(render, NULL, caps[i] ? shown : NULL, caps[i], &r);
        CHECK(st == (i == 3 ? LF_OK : LF_TRUNCATED) && r.required == 6 &&
              r.written == written[i] && r.terminated == (i != 0) &&
              r.complete == (i == 3));
        memset(shown, '#', 16);
        r.required = 99;
        st = lf_transaction(render, NULL, caps[i] ? shown : NULL, caps[i], &r);
        CHECK(st == (i == 3 ? LF_OK : LF_CAPACITY));
        if (i < 3)
            CHECK(shown[0] == '#' && r.required == 99);
    }
    for (size_t width = 0; width <= 8; ++width)
        for (size_t length = 0; length <= 6; ++length)
        {
            unsigned char record[10];
            memset(record, 99, sizeof record);
            enum lb_status st = lb_record((const unsigned char *)"abcdef",
                                          length, width ? record : NULL, width);
            if (length > width)
            {
                CHECK(st == LB_FULL);
                for (unsigned j = 0; j < 10; ++j)
                    CHECK(record[j] == 99);
            }
            else
            {
                CHECK(st == LB_OK);
                for (size_t j = 0; j < length; ++j)
                    CHECK(record[j] == "abcdef"[j]);
                for (size_t j = length; j < width; ++j)
                    CHECK(record[j] == 0);
                for (size_t j = width; j < 10; ++j)
                    CHECK(record[j] == 99);
            }
        }
    printf(
        "BYTES-PASS:cases=%zu:move:spans:display:actual-cursor:actual-format:"
        "record\n",
        cases);
    return 0;
}
