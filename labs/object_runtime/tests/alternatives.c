#include "object_runtime_alternatives.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "OR_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int dictionary(void)
{
    struct or_dictionary d = {0}, before;
    int32_t out = 777;
    unsigned i;
    CHECK(or_dictionary_get(&d, 1, &out) == OR_MISS && out == 777);
    for (i = 1; i <= 4; i++)
        CHECK(or_dictionary_set(&d, (uint8_t)i, (int32_t)(i * 10)) == OR_OK);
    CHECK(or_dictionary_set(&d, 3, 93) == OR_OK && d.count == 4);
    memcpy(&before, &d, sizeof d);
    CHECK(or_dictionary_set(&d, 5, 50) == OR_FULL &&
          !memcmp(&before, &d, sizeof d));
    CHECK(or_dictionary_delete(&d, 1) == OR_OK);
    CHECK(or_dictionary_get(&d, 2, &out) == OR_OK && out == 20);
    CHECK(or_dictionary_get(&d, 3, &out) == OR_OK && out == 93);
    CHECK(or_dictionary_set(&d, 5, 50) == OR_OK);
    CHECK(or_dictionary_delete(&d, 3) == OR_OK &&
          or_dictionary_delete(&d, 5) == OR_OK &&
          or_dictionary_delete(&d, 2) == OR_OK &&
          or_dictionary_delete(&d, 4) == OR_OK && d.count == 0);
    memcpy(&before, &d, sizeof d);
    CHECK(or_dictionary_delete(&d, 2) == OR_MISS &&
          !memcmp(&before, &d, sizeof d));
    out = 777;
    CHECK(or_dictionary_get(&d, 0, &out) == OR_INVALID && out == 777);
    return 0;
}

static int generic(void)
{
    struct or_shapes s;
    struct or_object a = {0}, b = {0};
    struct or_cache cache = {0};
    int32_t out = 777, cached = 777;
    unsigned k;
    or_shapes_init(&s);
    CHECK(or_property_set(&s, &a, 1, 10) == OR_OK &&
          or_property_set(&s, &a, 2, 20) == OR_OK);
    CHECK(or_property_set(&s, &b, 2, 30) == OR_OK &&
          or_property_set(&s, &b, 1, 40) == OR_OK);
    CHECK(or_property_generic(&s, &b, 1, &out) == OR_OK && out == 40);
    out = 777;
    CHECK(or_property_generic(&s, &b, 3, &out) == OR_MISS && out == 777);
    for (k = 0; k < 1000; k++)
    {
        struct or_object *o = k % 2 ? &a : &b;
        uint8_t key = (uint8_t)(k % 3 + 1);
        enum or_status x, y;
        out = cached = 777;
        if (k % 7 == 0)
            CHECK(or_property_set(&s, o, 1, (int32_t)k) == OR_OK);
        x = or_property_generic(&s, o, key, &out);
        y = or_property_get(&s, o, key, &cache, &cached);
        CHECK(x == y && out == cached);
    }
    CHECK(cache.mega);
    return 0;
}

static int oracle(void)
{
    unsigned seq, step, k;
    for (seq = 0; seq < 7776; seq++)
    {
        unsigned code = seq;
        struct or_dictionary d = {0};
        for (step = 0; step < 5; step++)
        {
            unsigned operation = code % 6;
            enum or_status status;
            code /= 6;
            if (operation < 4)
                status =
                    or_dictionary_set(&d, (uint8_t)(operation + 1),
                                      (int32_t)(10 * (operation + 1) + step));
            else
                status = or_dictionary_delete(&d, (uint8_t)(operation - 3));
            printf("D %u %u %u %d %u", seq, step, operation, status, d.count);
            for (k = 1; k <= 5; k++)
            {
                int32_t out = 777;
                enum or_status found = or_dictionary_get(&d, (uint8_t)k, &out);
                printf(" %d %d", found, out);
            }
            puts("");
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "dictionary"))
        return dictionary();
    if (!strcmp(argv[1], "generic"))
        return generic();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    return 2;
}
