/*
 * Public contract tests for the type-interface lab (types.h).
 *
 * Every check that fails prints "TD_CHECK:<line>:<expression>" to stderr and
 * exits with status 1, so a rejected learner build names the first broken
 * promise. The campaign matches fragments of those expressions; keep them
 * spelled as written.
 */
#include "types.h"
#include <stdio.h>
#include <stdlib.h>

#define TD_REQUIRE(condition)                                          \
    do                                                                 \
    {                                                                  \
        if (!(condition))                                              \
        {                                                              \
            fprintf(stderr, "TD_CHECK:%d:%s\n", __LINE__, #condition); \
            exit(1);                                                   \
        }                                                              \
    } while (0)

/*
 * Redeclared without the td_unary aliases: both spellings denote the same
 * function type, so the compiler must accept them together.
 */
int (*td_factory(void))(int);

/* Value stored in an output before a call that must leave it untouched. */
enum
{
    UNTOUCHED = 91,
    UNTOUCHED_DISTANCE = 92,
    ROW_WIDTH = 4
};

static void check_row_factory(void)
{
    int (*row)[ROW_WIDTH] = td_row_factory();
    TD_REQUIRE(row && (*row)[0] == 1 && (*row)[3] == 4);

    /* The pointer-to-array and the array-of-pointers views of the same row. */
    int *elements[ROW_WIDTH] = {&(*row)[0], &(*row)[1], &(*row)[2], &(*row)[3]};
    TD_REQUIRE(*elements[2] == 3 && sizeof *row == ROW_WIDTH * sizeof(int));
}

static void check_function_factories(void)
{
    int (*function)(int) = td_factory();
    struct td_callable wrapped = td_wrapped_factory();
    TD_REQUIRE(function && wrapped.call && function(7) == 14 &&
               wrapped.call(7) == 14);
}

static void check_opaque_owners(void)
{
    /* Two distinct owners; a third index is out of range. */
    struct td_opaque *a = td_owner(0);
    struct td_opaque *b = td_owner(1);
    int a_value = UNTOUCHED;
    int b_value = UNTOUCHED_DISTANCE;

    TD_REQUIRE(a && b && a != b && !td_owner(2));
    TD_REQUIRE(td_add(a, 7) == TD_OK && td_get(a, &a_value) == TD_OK &&
               a_value == 7);
    TD_REQUIRE(td_get(b, &b_value) == TD_OK && b_value == 0);
    /* A rejected addition leaves the stored value unchanged. */
    TD_REQUIRE(td_add(a, 1000) == TD_RANGE && td_get(a, &a_value) == TD_OK &&
               a_value == 7);
}

static void check_exact_units(void)
{
    struct td_speed speed = {UNTOUCHED};
    struct td_meters distance = {UNTOUCHED_DISTANCE};

    TD_REQUIRE(td_speed((struct td_meters){12}, (struct td_seconds){3},
                        &speed) == TD_OK &&
               speed.value == 4);

    /* Inexact division and zero duration are rejected, output untouched. */
    speed.value = UNTOUCHED;
    TD_REQUIRE(td_speed((struct td_meters){13}, (struct td_seconds){3},
                        &speed) == TD_RANGE &&
               speed.value == UNTOUCHED);
    TD_REQUIRE(td_speed((struct td_meters){1}, (struct td_seconds){0},
                        &speed) == TD_RANGE &&
               speed.value == UNTOUCHED);

    TD_REQUIRE(td_distance((struct td_speed){4}, (struct td_seconds){3},
                           &distance) == TD_OK &&
               distance.value == 12);
    TD_REQUIRE(td_distance((struct td_speed){1000}, (struct td_seconds){2},
                           &distance) == TD_RANGE &&
               distance.value == 12);
}

static void representations(void)
{
    check_row_factory();
    check_function_factories();
    check_opaque_owners();
    check_exact_units();
    puts("TD_REP_PASS row_factory function_alias wrapper opaque_distinct "
         "exact_units");
}

static void calls(void)
{
    const int rows[2][ROW_WIDTH] = {{1, 2, 3, 4}, {5, 6, 7, 8}};
    struct td_context context = {1, 0};
    int out = UNTOUCHED;

    /* Eight elements each bumped by the bias: sum 36 + 8 = 44, 8 calls. */
    TD_REQUIRE(td_rows(rows, 2, td_adapter, &context, &out) == TD_OK &&
               out == 44 && context.calls == 8);
    TD_REQUIRE(td_rows(NULL, 0, NULL, NULL, &out) == TD_OK && out == 0);

    out = UNTOUCHED;
    TD_REQUIRE(td_rows(rows, 2, NULL, &context, &out) == TD_INVALID &&
               out == UNTOUCHED && context.calls == 8);

    /* One out-of-range element rejects the whole batch before any callback. */
    const int invalid[2][ROW_WIDTH] = {{1, 2, 3, 4}, {5, 6, 7, 1001}};
    TD_REQUIRE(td_rows(invalid, 2, td_adapter, &context, &out) == TD_RANGE &&
               out == UNTOUCHED && context.calls == 8);
    puts("TD_CALL_PASS actual_row_stride admission_before_callbacks "
         "explicit_adapter");
}

/* Callback context that records what a running job allows. */
struct td_reenter
{
    struct td_job *job;
    enum td_status cancel;
    enum td_status reset;
    enum td_status reenter;
    unsigned calls;
};

/* Calls back into the job API while the job is RUNNING; all must be BUSY. */
static int reenter(void *opaque, int value)
{
    struct td_reenter *state = opaque;
    int nested_out = UNTOUCHED;

    ++state->calls;
    state->cancel = td_cancel(state->job);
    state->reset = td_reset(state->job);
    state->reenter = td_run(state->job, &nested_out);
    TD_REQUIRE(nested_out == UNTOUCHED);
    return value + 1;
}

static void retained(void)
{
    struct td_context context = {2, 0};
    struct td_job job = TD_JOB_INIT;
    int out = UNTOUCHED;

    TD_REQUIRE(td_submit(&job, td_adapter, &context, 5) == TD_OK &&
               context.calls == 0);
    TD_REQUIRE(td_reset(&job) == TD_BUSY &&
               td_submit(&job, td_adapter, &context, 9) == TD_STATE);

    /* Cancel drops the borrowed callback and context so run cannot use them. */
    TD_REQUIRE(td_cancel(&job) == TD_OK && job.state == TD_CANCELED &&
               !job.callback && !job.context);
    TD_REQUIRE(td_run(&job, &out) == TD_STATE && out == UNTOUCHED &&
               context.calls == 0);

    TD_REQUIRE(td_reset(&job) == TD_OK &&
               td_submit(&job, td_adapter, &context, 5) == TD_OK);
    TD_REQUIRE(td_run(&job, &out) == TD_OK && out == 7 && context.calls == 1 &&
               job.state == TD_DONE);
    TD_REQUIRE(td_run(&job, &out) == TD_STATE && context.calls == 1);
    TD_REQUIRE(td_reset(&job) == TD_OK);

    struct td_reenter reentry = {&job, TD_OK, TD_OK, TD_OK, 0};
    TD_REQUIRE(td_submit(&job, reenter, &reentry, 5) == TD_OK &&
               td_run(&job, &out) == TD_OK && out == 6);
    TD_REQUIRE(reentry.calls == 1 && reentry.cancel == TD_BUSY &&
               reentry.reset == TD_BUSY && reentry.reenter == TD_BUSY);
    puts("TD_JOB_PASS borrowed_until_quiescent cancel_suppresses_call "
         "running_busy");
}

int main(void)
{
    representations();
    calls();
    retained();
    puts("TD_PASS typed_interfaces");
    return 0;
}
