#include "algorithms.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                                           \
    do                                                                     \
    {                                                                      \
        if (!(x))                                                          \
        {                                                                  \
            fprintf(stderr, "ALGORITHM_ASSERT bytes_numbers line%d: %s\n", \
                    __LINE__, #x);                                         \
            return 1;                                                      \
        }                                                                  \
    } while (0)

int main(void)
{
    unsigned char input[] = {44, 65, 44, 44}, output[64] = {0};
    struct ad_span parts[65];
    size_t count = 99;
    CHECK(ad_byte_split(input, 4, 44, 1, parts, 65, &count) == AD_OK &&
          count == 4);
    size_t lengths[] = {0, 1, 0, 0}, offsets[] = {0, 1, 3, 4};
    for (unsigned i = 0; i < 4; ++i)
    {
        CHECK(parts[i].length == lengths[i] && parts[i].offset == offsets[i]);
    }
    CHECK(ad_byte_split(input, 4, 44, 0, parts, 65, &count) == AD_OK &&
          count == 1 && parts[0].offset == 1 && parts[0].length == 1);
    struct ad_span old[65];
    memcpy(old, parts, sizeof old);
    count = 99;
    CHECK(ad_byte_split(input, 4, 44, 1, parts, 3, &count) == AD_SPACE &&
          count == 99 && !memcmp(old, parts, sizeof old));
    CHECK(ad_byte_split(NULL, 0, 0, 1, parts, 1, &count) == AD_OK &&
          count == 1 && !parts[0].length);
    CHECK(ad_byte_split(NULL, 0, 0, 0, NULL, 0, &count) == AD_OK && !count);
    CHECK(ad_byte_copy(output, 4, input, 4) == AD_OK &&
          !memcmp(output, input, 4));
    output[0] = 99;
    CHECK(ad_byte_copy(output, 3, input, 4) == AD_SPACE && output[0] == 99);
    CHECK(ad_byte_copy(NULL, 0, NULL, 0) == AD_OK);
    unsigned char arena[] = {1, 2, 3, 4, 5}, moved[] = {1, 1, 2, 3, 4};
    CHECK(ad_byte_move(arena, 5, 1, 0, 4) == AD_OK && !memcmp(arena, moved, 5));
    CHECK(ad_byte_move(arena, 5, 2, 0, 4) == AD_INVALID &&
          !memcmp(arena, moved, 5));
    CHECK(ad_byte_move(NULL, 0, 0, 0, 0) == AD_OK);
    for (unsigned a = 0; a < 256; ++a)
    {
        for (unsigned b = 0; b < 256; ++b)
        {
            unsigned char x = (unsigned char)a, y = (unsigned char)b;
            int relation = 99;
            CHECK(ad_byte_compare(&x, 1, &y, 1, &relation) == AD_OK &&
                  relation == (a < b ? -1 : a > b));
        }
    }
    int relation;
    CHECK(ad_byte_compare(NULL, 0, input, 4, &relation) == AD_OK &&
          relation < 0);
    CHECK(ad_byte_find(input, 4, 65, &count) == AD_OK && count == 1);
    CHECK(ad_byte_find(input, 4, 66, &count) == AD_OK && count == 4);
    unsigned char spaced[] = {32, 9, 'A', 0, 'B', 13, 32};
    struct ad_span trimmed;
    CHECK(ad_byte_trim(spaced, 7, &trimmed) == AD_OK && trimmed.offset == 2 &&
          trimmed.length == 3);
    CHECK(ad_byte_trim_in_place(spaced, 7, &count) == AD_OK && count == 3 &&
          spaced[0] == 'A' && spaced[1] == 0 && spaced[2] == 'B');
    CHECK(ad_byte_trim(NULL, 0, &trimmed) == AD_OK && !trimmed.length &&
          !trimmed.offset);

    struct
    {
        const char *text;
        enum ad_status status;
        int32_t value;
    } cases[] = {{"-2147483648", AD_OK, INT32_MIN},
                 {"2147483647", AD_OK, INT32_MAX},
                 {"2147483648", AD_OVERFLOW, 0},
                 {"-2147483649", AD_OVERFLOW, 0},
                 {"+0", AD_OK, 0},
                 {"-0", AD_OK, 0},
                 {"12x", AD_INVALID, 0},
                 {"", AD_INVALID, 0},
                 {" 1", AD_INVALID, 0},
                 {"1 ", AD_INVALID, 0},
                 {"+", AD_INVALID, 0}};

    for (size_t i = 0; i < sizeof cases / sizeof *cases; ++i)
    {
        int32_t value = 77;
        count = 88;
        enum ad_status rc =
            ad_parse_i32((const unsigned char *)cases[i].text,
                         strlen(cases[i].text), 10, 0, 1, &value, &count);
        CHECK(rc == cases[i].status);
        if (rc == AD_OK)
        {
            CHECK(value == cases[i].value && count == strlen(cases[i].text));
        }
        else
        {
            CHECK(value == 77 && count == 88);
        }
    }
    int32_t value;
    unsigned char field[] = {'1', '2', '3', '4'};
    CHECK(ad_parse_i32(field, 2, 10, 0, 1, &value, &count) == AD_OK &&
          value == 12 && count == 2);
    CHECK(ad_parse_i32((const unsigned char *)"12x", 3, 10, 0, 0, &value,
                       &count) == AD_OK &&
          value == 12 && count == 2);
    CHECK(ad_parse_i32((const unsigned char *)"0x10", 4, 16, 1, 1, &value,
                       &count) == AD_OK &&
          value == 16);
    CHECK(ad_parse_i32((const unsigned char *)"0x", 2, 16, 1, 1, &value,
                       &count) == AD_INVALID);
    uint32_t unsigned_value;
    CHECK(ad_parse_u32((const unsigned char *)"4294967295", 10, 10, 0, 1,
                       &unsigned_value, &count) == AD_OK &&
          unsigned_value == UINT32_MAX);
    CHECK(ad_parse_u32((const unsigned char *)"-0", 2, 10, 0, 1,
                       &unsigned_value, &count) == AD_INVALID);
    size_t formats = 0;
    for (unsigned base = 2; base <= 16; ++base)
    {
        for (int32_t x = -1024; x <= 1024; ++x)
        {
            CHECK(ad_format_i32(x, base, output, 64, &count) == AD_OK);
            size_t used;
            CHECK(ad_parse_i32(output, count, base, 0, 1, &value, &used) ==
                      AD_OK &&
                  value == x && used == count);
            ++formats;
        }
    }
    int32_t endpoints[] = {INT32_MIN, INT32_MAX};
    for (unsigned j = 0; j < 2; ++j)
    {
        for (unsigned base = 2; base <= 16; ++base)
        {
            CHECK(ad_format_i32(endpoints[j], base, output, 64, &count) ==
                  AD_OK);
            CHECK(ad_parse_i32(output, count, base, 0, 1, &value, &count) ==
                      AD_OK &&
                  value == endpoints[j]);
        }
    }
    memset(output, 99, sizeof output);
    count = 88;
    CHECK(ad_format_i32(INT32_MIN, 10, output, 10, &count) == AD_SPACE &&
          count == 88);
    for (unsigned i = 0; i < 64; ++i)
    {
        CHECK(output[i] == 99);
    }
    printf(
        "ALGORITHM_BYTES_NUMBERS_PASS unsigned_pairs=65536 formats=%zu exact_boundaries one_way_T11_import\n",
        formats);
    return 0;
}
