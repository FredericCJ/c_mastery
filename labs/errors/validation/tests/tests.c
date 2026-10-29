#include "validation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "ER_CHECK EVV line%d: %s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)

static struct evv_input canary(void)
{
    struct evv_input v = {0};
    v.count = 99;
    v.limit = 98;
    for (unsigned i = 0; i < 3; ++i)
        v.item[i] = (er_config){91, 92, 93};
    return v;
}

static void fill(struct evv_diagnostic *d)
{
    for (unsigned i = 0; i < 10; ++i)
        d[i] = (struct evv_diagnostic){91, 92, {93, 94, 95}};
}

static int untouched(struct evv_input v)
{
    return v.count == 99 && v.limit == 98 && v.item[0].age == 91 &&
           v.item[2].denominator == 93;
}

static int equal_config(er_config a, er_config b)
{
    return a.age == b.age && a.quota == b.quota &&
           a.denominator == b.denominator;
}

static int base(void)
{
    er_config out = {91, 92, 93};
    unsigned errors[4] = {81, 82, 83, 84};
    er_validation r = er_validate((er_config){-1, 1025, 0}, &out, errors, 2);
    CHECK(r.total == 3 && r.stored == 2 && r.truncated == 1 && errors[0] == 1 &&
          errors[1] == 2 && errors[2] == 83 &&
          equal_config(out, (er_config){91, 92, 93}));
    r = er_validate((er_config){150, 1024, 1}, &out, NULL, 0);
    CHECK(r.total == 0 && r.stored == 0 &&
          equal_config(out, (er_config){150, 1024, 1}));
    r = er_validate((er_config){0, 0, 1}, NULL, errors, 2);
    CHECK(r.total == SIZE_MAX && r.stored == 0 && errors[2] == 83);
    puts(
        "EVV_BASE actual-er-validate all-fields prefix-not-validity transactional-output");
    return 0;
}

static int ratio(void)
{
    struct evv_input i = {0}, out = canary();
    struct evv_diagnostic errors[10];
    fill(errors);
    i.count = 1;
    i.limit = 1;
    i.item[0] = (er_config){0, 3, 2};
    struct evv_summary r = evv_validate(&i, 7, &out, errors, 9);
    CHECK(r.total == 1 && r.ratio_checks == 1 && errors[0].code == 4 &&
          untouched(out));
    i.item[0] = (er_config){-1, 3, 2};
    r = evv_validate(&i, 7, &out, errors, 9);
    CHECK(r.total == 2 && r.ratio_checks == 1 && errors[0].code == 1 &&
          errors[1].code == 4);
    i.item[0] = (er_config){-1, 1025, 0};
    r = evv_validate(&i, 7, &out, errors, 9);
    CHECK(r.total == 3 && r.ratio_checks == 0 && untouched(out));
    i.item[0] = (er_config){0, 1024, 4096};
    i.limit = UINT32_MAX;
    r = evv_validate(&i, 7, &out, errors, 0);
    CHECK(r.total == 0 && r.ratio_checks == 1 && out.limit == UINT32_MAX &&
          equal_config(out.item[0], i.item[0]));
    i.item[0] = (er_config){0, 0, 1};
    i.limit = 0;
    r = evv_validate(&i, 7, &out, errors, 0);
    CHECK(r.total == 0);
    puts(
        "EVV_RATIO admitted-denominator exact-rational remainder independent-age ordered-phases");
    return 0;
}

static int nested(void)
{
    struct evv_input i = {0}, out = canary();
    i.count = 3;
    i.limit = 1;
    i.item[0] = (er_config){-1, 3, 2};
    i.item[1] = (er_config){151, 1025, 0};
    i.item[2] = (er_config){0, 4, 3};
    struct evv_diagnostic d[10];
    fill(d);
    unsigned ids[6] = {1, 1, 2, 3, 4, 4}, items[6] = {0, 1, 1, 1, 0, 2};
    for (size_t cap = 0; cap <= 9; ++cap)
    {
        fill(d);
        struct evv_summary r = evv_validate(&i, 42, &out, cap ? d : NULL, cap);
        CHECK(r.total == 6 && r.stored == (cap < 6 ? cap : 6) &&
              r.truncated == (cap < 6) && r.ratio_checks == 2 &&
              untouched(out));
        for (size_t k = 0; k < r.stored; ++k)
            CHECK(d[k].code == ids[k] && d[k].depth == 3 &&
                  d[k].path[0] == 42 && d[k].path[1] == items[k] &&
                  d[k].path[2] == ids[k]);
        CHECK(d[r.stored].code == 91 && d[r.stored].path[2] == 95);
    }
    puts(
        "EVV_NESTED full-path root-item-field independent-prefix truncation-deterministic");
    return 0;
}

static int admission(void)
{
    struct evv_input i = {0}, out = canary();
    struct evv_diagnostic d[10];
    fill(d);
    struct evv_summary r;
    r = evv_validate(&i, 1, &out, d, 10);
    CHECK(r.total == SIZE_MAX && r.stored == 0 && r.truncated == 0 &&
          r.ratio_checks == 0 && untouched(out) && d[0].code == 91);
    r = evv_validate(NULL, 1, &out, d, 9);
    CHECK(r.total == SIZE_MAX && untouched(out));
    r = evv_validate(&i, 0, &out, d, 9);
    CHECK(r.total == SIZE_MAX && untouched(out));
    r = evv_validate(&i, 1, &out, NULL, 1);
    CHECK(r.total == SIZE_MAX && untouched(out));
    r = evv_validate(&i, 1, NULL, d, 1);
    CHECK(r.total == SIZE_MAX && d[0].code == 91);
    i.count = 4;
    r = evv_validate(&i, 1, &out, d, 9);
    CHECK(r.total == SIZE_MAX && untouched(out));
    i.count = 0;
    r = evv_validate(&i, 1, &out, NULL, 0);
    CHECK(r.total == 0 && r.stored == 0 && r.ratio_checks == 0 &&
          out.count == 0);
    puts(
        "EVV_ADMISSION null-zero count-empty structural-refusal untouched-diagnostics");
    return 0;
}

static int security(void)
{
    struct evv_input i = {0}, live = canary();
    i.count = 1;
    i.item[0] = (er_config){-1, 1025, 0};
    struct evv_summary r = evv_validate(&i, 1, &live, NULL, 0);
    CHECK(r.total == 3 && r.stored == 0 && r.truncated == 1 && untouched(live));
    i.item[0] = (er_config){30, 2, 2};
    i.limit = 1;
    r = evv_validate(&i, 1, &live, NULL, 0);
    CHECK(r.total == 0 && live.count == 1 &&
          equal_config(live.item[0], i.item[0]));
    puts(
        "EVV_SECURITY original-local-config attacker-fields no-invalid-publication repaired-valid-control");
    return 0;
}

static int oracle(void)
{
    char kind;
    size_t n, cap;
    unsigned root;
    uint32_t limit;
    while (scanf(" %c %zu %zu %u %u", &kind, &n, &cap, &root, &limit) == 5)
    {
        struct evv_input i = {0}, out = canary();
        i.count = n;
        i.limit = limit;
        for (unsigned k = 0; k < 3; ++k)
            if (scanf("%d %d %d", &i.item[k].age, &i.item[k].quota,
                      &i.item[k].denominator) != 3)
                return 2;
        if (kind == 'B')
        {
            unsigned e[4] = {91, 91, 91, 91};
            er_config c = {91, 92, 93};
            er_validation r = er_validate(i.item[0], &c, cap ? e : NULL, cap);
            printf("%zu %zu %d %d %d %d %u %u %u %u\n", r.total, r.stored,
                   r.truncated, c.age, c.quota, c.denominator, e[0], e[1], e[2],
                   e[3]);
        }
        else if (kind == 'V')
        {
            struct evv_diagnostic e[10];
            fill(e);
            struct evv_summary r =
                evv_validate(&i, root, &out, cap ? e : NULL, cap);
            printf("%zu %zu %u %u %zu %u", r.total, r.stored, r.truncated,
                   r.ratio_checks, out.count, out.limit);
            for (unsigned k = 0; k < 3; ++k)
                printf(" %d %d %d", out.item[k].age, out.item[k].quota,
                       out.item[k].denominator);
            for (unsigned k = 0; k < 10; ++k)
                printf(" %u %u %u %u %u", e[k].code, e[k].depth, e[k].path[0],
                       e[k].path[1], e[k].path[2]);
            putchar('\n');
        }
        else
            return 2;
    }
    return ferror(stdin) ? 2 : 0;
}

static int metrics(void)
{
    double times[7];
    size_t checksum = 0;
    for (unsigned sample = 0; sample < 7; ++sample)
    {
        clock_t start = clock();
        for (unsigned k = 0; k < 10000; ++k)
        {
            struct evv_input i = {0}, o = canary();
            i.count = 3;
            i.limit = 1;
            i.item[0] = (er_config){-1, 3, 2};
            i.item[1] = (er_config){151, 1025, 0};
            i.item[2] = (er_config){0, 4, 3};
            struct evv_diagnostic d[9];
            struct evv_summary r = evv_validate(&i, 42, &o, d, 9);
            CHECK(r.total == 6 && r.ratio_checks == 2 && untouched(o));
            checksum += r.total + r.stored + r.ratio_checks;
        }
        times[sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
    }
    printf(
        "{\"iterations\":10000,\"checksum\":%zu,\"allocation_calls\":0,\"input_size\":%zu,\"diagnostic_size\":%zu,\"summary_size\":%zu,\"seconds\":[",
        checksum, sizeof(struct evv_input), sizeof(struct evv_diagnostic),
        sizeof(struct evv_summary));
    for (unsigned i = 0; i < 7; ++i)
        printf("%s%.9f", i ? "," : "", times[i]);
    puts("]}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "validate_base"))
        return base();
    if (!strcmp(argv[1], "validate_ratio"))
        return ratio();
    if (!strcmp(argv[1], "validate_nested"))
        return nested();
    if (!strcmp(argv[1], "validate_admission"))
        return admission();
    if (!strcmp(argv[1], "validate_security"))
        return security();
    if (!strcmp(argv[1], "oracle"))
        return oracle();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
