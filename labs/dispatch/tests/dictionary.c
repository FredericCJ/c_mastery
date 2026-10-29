#include "dispatch_dictionary.h"
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *test_name = "arguments";
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "DS_CHECK:%s:%d:%s\n", test_name, __LINE__, #x); \
            exit(1);                                                         \
        }                                                                    \
    } while (0)

struct ds_event
{
    int a, b, status;
};
static struct ds_event events[16];
static unsigned calls, policy;

static int operation(int a, int b, int *out)
{
    CHECK(calls < 16);
    int status = DS_OK;
    long long value = policy == 1 ? (long long)a - b : (long long)a + b;
    if (policy == 2 && b < 0)
    {
        status = DS_FAILED;
    }
    else if (value < INT_MIN || value > INT_MAX)
    {
        status = DS_RANGE;
    }
    events[calls++] = (struct ds_event){a, b, status};
    *out = status ? 999 : (int)value;
    return status;
}

static void dictionary(int emit)
{
    unsigned cases = 0;
    const int values[3] = {-2, 0, 3};
    for (size_t n = 0; n <= 5; ++n)
    {
        unsigned count = 1;
        for (size_t i = 0; i < n; ++i)
        {
            count *= 3;
        }
        for (unsigned code = 0; code < count; ++code)
        {
            for (policy = 0; policy < 3; ++policy)
            {
                unsigned script = code;
                int input[5];
                for (size_t i = 0; i < n; ++i)
                {
                    input[i] = values[script % 3];
                    script /= 3;
                }
                calls = 0;
                int out = 91, status = ds_fold(input, n, 1, operation, &out),
                    expected = 1, want = DS_OK;
                unsigned used = 0;
                for (size_t i = 0; i < n; ++i)
                {
                    int next =
                        policy == 1 ? expected - input[i] : expected + input[i];
                    int step = policy == 2 && input[i] < 0 ? DS_FAILED : DS_OK;
                    CHECK(events[i].a == expected && events[i].b == input[i] &&
                          events[i].status == step);
                    ++used;
                    if (step)
                    {
                        want = step;
                        break;
                    }
                    expected = next;
                }
                CHECK(status == want && calls == used &&
                      out == (want ? 91 : expected));
                if (policy == 0)
                {
                    int direct = 91;
                    CHECK(ds_sum(input, n, 1, &direct) == DS_OK &&
                          direct == out);
                }
                if (emit)
                {
                    printf("D %zu %u %u %d %d %u", n, code, policy, status, out,
                           calls);
                    for (unsigned i = 0; i < calls; ++i)
                    {
                        printf(" %d %d %d", events[i].a, events[i].b,
                               events[i].status);
                    }
                    puts("");
                }
                ++cases;
            }
        }
    }
    CHECK(cases == 1092);
    policy = 0;
    calls = 0;
    int input[3] = {INT_MAX, 1, -1}, out = 91;
    CHECK(ds_fold(input, 3, 0, operation, &out) == DS_RANGE && out == 91 &&
          calls == 2 && events[0].a == 0 && events[0].b == INT_MAX &&
          events[1].a == INT_MAX && events[1].b == 1);
    CHECK(ds_sum(input, 3, 0, &out) == DS_RANGE && out == 91);
    calls = 0;
    CHECK(ds_fold(NULL, 0, 1, operation, &out) == DS_OK && out == 1 && !calls);
}

struct ds_job
{
    const int *values;
    size_t count;
    int result, status;
};

static void *worker(void *p)
{
    struct ds_job *job = p;
    job->status = ds_sat_tree(job->values, job->count, &job->result);
    return NULL;
}

static int scalar(int a, int b)
{
    long long value = (long long)a + b;
    return value < INT_MIN ? INT_MIN : value > INT_MAX ? INT_MAX : (int)value;
}

static void reduction(void)
{
    const int edge[7] = {INT_MIN, INT_MIN + 1, -1, 0, 1, INT_MAX - 1, INT_MAX};
    for (unsigned i = 0; i < 7; ++i)
    {
        for (unsigned j = 0; j < 7; ++j)
        {
            int out = 91;
            CHECK(ds_sat_add(edge[i], edge[j], &out) == DS_OK &&
                  out == scalar(edge[i], edge[j]));
        }
    }
    int input[3] = {INT_MAX, 1, -1}, left = 91, tree = 91;
    CHECK(ds_sat_left(input, 3, &left) == DS_OK && left == INT_MAX - 1);
    CHECK(ds_sat_tree(input, 3, &tree) == DS_OK && tree == INT_MAX &&
          left != tree);
    struct ds_job jobs[2] = {{input, 1, 91, DS_FAILED},
                             {input + 1, 2, 91, DS_FAILED}};
    pthread_t threads[2];
    CHECK(pthread_create(&threads[0], NULL, worker, &jobs[0]) == 0 &&
          pthread_create(&threads[1], NULL, worker, &jobs[1]) == 0);
    CHECK(pthread_join(threads[0], NULL) == 0 &&
          pthread_join(threads[1], NULL) == 0);
    CHECK(jobs[0].status == DS_OK && jobs[1].status == DS_OK);
    int joined = 91;
    CHECK(ds_sat_add(jobs[0].result, jobs[1].result, &joined) == DS_OK &&
          joined == tree && joined != left);
    const int positive[4] = {1, 2, 3, 4};
    CHECK(ds_sat_left(positive, 4, &left) == DS_OK &&
          ds_sat_tree(positive, 4, &tree) == DS_OK && left == 10 && tree == 10);
    CHECK(ds_sat_left(NULL, 0, &left) == DS_OK && left == 0);
    int out = 91;
    CHECK(ds_sat_tree(NULL, 1, &out) == DS_INVALID && out == 91);
}

/* A well-typed erased callback may change the claimed type constructor. The
   tag test is a runtime law, unlike kind checking over an abstract F. */
struct raw_value
{
    unsigned constructor;
    int value;
};
typedef struct raw_value (*erased_map_fn)(struct raw_value);

static struct raw_value same_constructor(struct raw_value x)
{
    ++x.value;
    return x;
}

static struct raw_value wrong_constructor(struct raw_value x)
{
    x.constructor ^= 1u;
    ++x.value;
    return x;
}

static void kinds(void)
{
    struct raw_value x = {0, 1};
    erased_map_fn good = same_constructor, bad = wrong_constructor;
    struct raw_value a = good(x), b = bad(x);
    CHECK(a.value == 2 && a.constructor == x.constructor);
    CHECK(b.value == 2 && b.constructor != x.constructor);
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    test_name = argv[1];
    if (!strcmp(argv[1], "reduction"))
    {
        dictionary(0);
        reduction();
        kinds();
    }
    else if (!strcmp(argv[1], "oracle"))
    {
        dictionary(1);
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
