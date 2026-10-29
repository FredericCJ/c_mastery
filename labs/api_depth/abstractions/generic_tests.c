#include "generic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "GENERIC-CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

struct ac_memory
{
    unsigned calls, owned, released, fail_at;
};

static void *allocate(void *ctx, size_t size, size_t alignment)
{
    struct ac_memory *m = ctx;
    ++m->calls;
    if (m->calls == m->fail_at || alignment > _Alignof(max_align_t))
    {
        return 0;
    }
    void *p = malloc(size);
    if (p)
    {
        ++m->owned;
    }
    return p;
}

static void release(void *ctx, void *p)
{
    struct ac_memory *m = ctx;
    ++m->released;
    free(p);
}

static unsigned evaluations;

static int value(void)
{
    ++evaluations;
    return 7;
}

int main(void)
{
    struct as_ints ints = {0};
    AsGenerated generated = {0};
    CHECK(as_generic_push(&ints, value()) == AS_OK && evaluations == 1);
    CHECK(AsGenerated_push(&generated, value()) == AS_OK && evaluations == 2);
    int erased_values[4] = {0}, stage_ints[4] = {0};
    struct as_erased erased = {erased_values,
                               0,
                               4,
                               {sizeof(int), _Alignof(int), as_int_copy_element,
                                as_int_drop_element, 0}};
    CHECK(as_erased_append(&erased, 0, 0, 0, 0, _Alignof(int)) == AS_OK &&
          erased.count == 0);
    int input = 7;
    CHECK(as_erased_append(&erased, &input, 1, stage_ints, 4, _Alignof(int)) ==
          AS_OK);
    CHECK(ints.values[0] == 7 && generated.values[0] == 7 &&
          erased_values[0] == 7);
    for (unsigned i = 1; i < 4; ++i)
    {
        CHECK(as_int_push(&ints, (int)i) == AS_OK &&
              AsGenerated_push(&generated, (int)i) == AS_OK);
        input = (int)i;
        CHECK(as_erased_append(&erased, &input, 1, stage_ints, 4,
                               _Alignof(int)) == AS_OK);
    }
    CHECK(as_int_push(&ints, 8) == AS_FULL &&
          AsGenerated_push(&generated, 8) == AS_FULL &&
          as_erased_append(&erased, &input, 1, stage_ints, 4, _Alignof(int)) ==
              AS_FULL);
    CHECK(!memcmp(ints.values, generated.values, sizeof ints.values) &&
          !memcmp(ints.values, erased_values, sizeof ints.values));
    unsigned char bytes[3] = {'a', 0, 'b'};
    struct as_value text, empty;
    CHECK(av_text(bytes, 3, &text) == AS_OK && av_valid(&text) == AS_OK &&
          text.as.text.length == 3 && text.as.text.data[1] == 0);
    CHECK(av_text(0, 0, &empty) == AS_OK && empty.tag == AV_TEXT);
    struct as_value invalid = {.tag = (enum av_tag)99};
    CHECK(av_valid(&invalid) == AS_INVALID && av_drop(&invalid) == AS_INVALID);
    struct as_value error = {.tag = AV_ERROR, .as.error = {-2, 2}},
                    integer = {.tag = AV_INT, .as.integer = 7};
    CHECK(av_valid(&error) == AS_OK && av_valid(&integer) == AS_OK);
    struct ac_memory m = {0};
    struct ac_allocate allocator = {&m, allocate, release};
    struct as_value sources[2], dest[4], stage[4];
    struct as_value preserved = integer;
    m.fail_at = 1;
    CHECK(av_owned(bytes, 3, allocator, &preserved) == AS_FAILED &&
          preserved.tag == AV_INT && preserved.as.integer == 7);
    m.fail_at = 0;
    CHECK(av_owned(bytes, 3, allocator, sources) == AS_OK &&
          av_owned(bytes, 3, allocator, sources + 1) == AS_OK);
    struct as_erased values = {
        dest,
        0,
        4,
        {sizeof(struct as_value), _Alignof(struct as_value),
         as_value_copy_element, as_value_drop_element, 0}};
    dest[0] = integer;
    values.count = 1;
    m.fail_at = m.calls + 2;
    CHECK(as_erased_append(&values, sources, 2, stage, 4,
                           _Alignof(struct as_value)) == AS_FAILED);
    CHECK(values.count == 1 && dest[0].tag == AV_INT &&
          dest[0].as.integer == 7 && m.released == 1 && m.owned == 3);
    m.fail_at = 0;
    CHECK(as_erased_append(&values, sources, 2, stage, 4,
                           _Alignof(struct as_value)) == AS_OK &&
          values.count == 3);
    CHECK(dest[1].as.owned.data != sources[0].as.owned.data &&
          dest[2].as.owned.data != sources[1].as.owned.data);
    sources[0].as.owned.data[0] = 'z';
    CHECK(dest[1].as.owned.data[0] == 'a');
    CHECK(as_erased_clear(&values) == AS_OK && values.count == 0);
    struct as_value moved = {.tag = AV_INT, .as.integer = 42};
    unsigned char *saved = sources[0].as.owned.data;
    CHECK(av_move(sources, &moved, 1) == AS_FAILED && moved.as.integer == 42 &&
          sources[0].as.owned.data == saved);
    CHECK(av_move(sources, &moved, 0) == AS_OK && sources[0].tag == AV_INT &&
          moved.as.owned.data == saved);
    CHECK(av_drop(&moved) == AS_OK && av_drop(sources + 1) == AS_OK &&
          m.owned == m.released);
    values.elements.alignment = 0;
    CHECK(as_erased_append(&values, &text, 1, stage, 4,
                           _Alignof(struct as_value)) == AS_INVALID);
    values.elements.alignment = _Alignof(struct as_value);
    values.elements.size = SIZE_MAX;
    CHECK(as_erased_append(&values, &text, 1, stage, 4,
                           _Alignof(struct as_value)) == AS_INVALID);
    values.elements.size = sizeof(struct as_value);
    CHECK(as_erased_append(&values, &text, 1, stage, 4, 0) == AS_INVALID);
    const char *tokens[] = {"",       "-",      "+",   "32767", "32768",
                            "-32768", "-32769", "1x2", "+0",    "-0"};
    const int wanted[] = {0, 0, 0, 32767, 0, -32768, 0, 0, 0, 0};
    const int codes[] = {-1, -1, -1, 0, -3, 0, -3, -2, 0, 0};
    const size_t offsets[] = {0, 1, 1, 0, 4, 0, 5, 1, 0, 0};
    for (unsigned i = 0; i < 10; ++i)
    {
        struct as_value v =
            av_parse((const unsigned char *)tokens[i], strlen(tokens[i]));
        CHECK(av_valid(&v) == AS_OK);
        if (codes[i])
        {
            CHECK(v.tag == AV_ERROR && v.as.error.code == codes[i] &&
                  v.as.error.offset == offsets[i]);
        }
        else
        {
            CHECK(v.tag == AV_INT && v.as.integer == wanted[i]);
        }
    }
    struct av_text fields[4] = {{(const unsigned char *)"x1", 2},
                                {(const unsigned char *)"32768", 5},
                                {(const unsigned char *)"-", 1},
                                {(const unsigned char *)"7", 1}};
    struct as_value results[4];
    CHECK(av_validate_fields(fields, 4, results) == AS_OK);
    CHECK(results[0].as.error.offset == 0 && results[1].as.error.offset == 4 &&
          results[2].as.error.offset == 1 && results[3].tag == AV_INT &&
          results[3].as.integer == 7);
    printf(
        "GENERIC-PASS:forms=3;capacity=4;staged_rollback=1;transfer_modes=2;tokens=10;fields=4;allocations=%u;frees=%u\n",
        m.owned, m.released);
    return 0;
}
