/* Executed defined examples. See analysis_only.c for excluded expressions. */
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define REQUIRE(x)    \
    do                \
    {                 \
        if (!(x))     \
            return 1; \
    } while (0)

struct tf_events
{
    char trace[8];
    unsigned n;
    int shared, acount, bcount;
};

static int emit(struct tf_events *s, char name, int value)
{
    s->trace[s->n++] = name;
    s->trace[s->n] = 0;
    return value;
}

static int bump(struct tf_events *s, char name)
{
    return emit(s, name, ++s->shared);
}

static int independent_a(struct tf_events *s)
{
    ++s->acount;
    return emit(s, 'A', 1);
}

static int independent_b(struct tf_events *s)
{
    ++s->bcount;
    return emit(s, 'B', 2);
}

static int pair(int a, int b)
{
    return 10 * a + b;
}

int main(void)
{
    struct tf_events s = {0};
    int value = emit(&s, 'A', 1) + emit(&s, 'B', 2) * emit(&s, 'C', 3);
    REQUIRE(value == 7 &&
            (strcmp(s.trace, "ABC") == 0 || strcmp(s.trace, "ACB") == 0 ||
             strcmp(s.trace, "BAC") == 0 || strcmp(s.trace, "BCA") == 0 ||
             strcmp(s.trace, "CAB") == 0 || strcmp(s.trace, "CBA") == 0));
    printf("TF_TREE arithmetic=7 trace=%s allowed=ABC,ACB,BAC,BCA,CAB,CBA\n",
           s.trace);
    s = (struct tf_events){0};
    value = emit(&s, 'A', 1) ? emit(&s, 'B', 2) : emit(&s, 'C', 3);
    REQUIRE(value == 2 && strcmp(s.trace, "AB") == 0);
    s = (struct tf_events){0};
    value = emit(&s, 'A', 0) ? emit(&s, 'B', 2) : emit(&s, 'C', 3);
    REQUIRE(value == 3 && strcmp(s.trace, "AC") == 0);
    s = (struct tf_events){0};
    int a = emit(&s, 'A', 1);
    int b = emit(&s, 'B', 2);
    int c = emit(&s, 'C', 3);
    value = a + b * c;
    REQUIRE(value == 7 && strcmp(s.trace, "ABC") == 0);
    puts("TF_TREE_PASS conditional=AB,AC temporaries=ABC");
    s = (struct tf_events){0};
    value = emit(&s, 'A', 0) && emit(&s, 'B', 2);
    REQUIRE(value == 0 && strcmp(s.trace, "A") == 0);
    s = (struct tf_events){0};
    value = emit(&s, 'A', 1) && emit(&s, 'B', 2);
    REQUIRE(value == 1 && strcmp(s.trace, "AB") == 0);
    s = (struct tf_events){0};
    value = (emit(&s, 'A', 1), emit(&s, 'B', 2));
    REQUIRE(value == 2 && strcmp(s.trace, "AB") == 0);
    s = (struct tf_events){0};
    value = pair(independent_a(&s), independent_b(&s));
    REQUIRE(value == 12 && s.acount == 1 && s.bcount == 1 &&
            (strcmp(s.trace, "AB") == 0 || strcmp(s.trace, "BA") == 0));
    printf("TF_INDEPENDENT trace=%s value=12\n", s.trace);
    s = (struct tf_events){0};
    value = pair(bump(&s, 'A'), bump(&s, 'B'));
    REQUIRE(s.shared == 2 && ((strcmp(s.trace, "AB") == 0 && value == 12) ||
                              (strcmp(s.trace, "BA") == 0 && value == 21)));
    printf("TF_SHARED trace=%s value=%d allowed=AB:12,BA:21\n", s.trace, value);
    /* Modification moved to an argument, but on two different objects: still defined. */
    int i = 0, j = 0;
    value = pair(i++, j++);
    REQUIRE(value == 0 && i == 1 && j == 1);
    puts(
        "TF_SEQUENCE_PASS short_circuit comma independent_bodies shared_bodies distinct_arguments");
    return 0;
}
