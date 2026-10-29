#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#define C(x)                                                   \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "TD_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)
/* These compatible declarations are deliberately spelled without aliases. */
int (*td_factory(void))(int);

static void representations(void)
{
    int (*row)[4] = td_row_factory();
    C(row && (*row)[0] == 1 && (*row)[3] == 4);
    int *p[4] = {&(*row)[0], &(*row)[1], &(*row)[2], &(*row)[3]};
    C(*p[2] == 3 && sizeof *row == 4 * sizeof(int));
    int (*fn)(int) = td_factory();
    struct td_callable wrapped = td_wrapped_factory();
    C(fn && wrapped.call && fn(7) == 14 && wrapped.call(7) == 14);
    struct td_opaque *a = td_owner(0), *b = td_owner(1);
    int x = 91, y = 92;
    C(a && b && a != b && !td_owner(2));
    C(td_add(a, 7) == TD_OK && td_get(a, &x) == TD_OK && x == 7);
    C(td_get(b, &y) == TD_OK && y == 0);
    C(td_add(a, 1000) == TD_RANGE && td_get(a, &x) == TD_OK && x == 7);
    struct td_speed speed = {91};
    struct td_meters distance = {92};
    C(td_speed((struct td_meters){12}, (struct td_seconds){3}, &speed) ==
          TD_OK &&
      speed.value == 4);
    speed.value = 91;
    C(td_speed((struct td_meters){13}, (struct td_seconds){3}, &speed) ==
          TD_RANGE &&
      speed.value == 91);
    C(td_speed((struct td_meters){1}, (struct td_seconds){0}, &speed) ==
          TD_RANGE &&
      speed.value == 91);
    C(td_distance((struct td_speed){4}, (struct td_seconds){3}, &distance) ==
          TD_OK &&
      distance.value == 12);
    C(td_distance((struct td_speed){1000}, (struct td_seconds){2}, &distance) ==
          TD_RANGE &&
      distance.value == 12);
    puts("TD_REP_PASS row_factory function_alias wrapper opaque_distinct "
         "exact_units");
}

static void calls(void)
{
    const int rows[2][4] = {{1, 2, 3, 4}, {5, 6, 7, 8}};
    struct td_context ctx = {1, 0};
    int out = 91;
    C(td_rows(rows, 2, td_adapter, &ctx, &out) == TD_OK && out == 44 &&
      ctx.calls == 8);
    C(td_rows(NULL, 0, NULL, NULL, &out) == TD_OK && out == 0);
    out = 91;
    C(td_rows(rows, 2, NULL, &ctx, &out) == TD_INVALID && out == 91 &&
      ctx.calls == 8);
    const int invalid[2][4] = {{1, 2, 3, 4}, {5, 6, 7, 1001}};
    C(td_rows(invalid, 2, td_adapter, &ctx, &out) == TD_RANGE && out == 91 &&
      ctx.calls == 8);
    puts("TD_CALL_PASS actual_row_stride admission_before_callbacks "
         "explicit_adapter");
}

struct td_reenter
{
    struct td_job *job;
    enum td_status cancel, reset, reenter;
    unsigned calls;
};

static int reenter(void *p, int value)
{
    struct td_reenter *r = p;
    int out = 91;
    ++r->calls;
    r->cancel = td_cancel(r->job);
    r->reset = td_reset(r->job);
    r->reenter = td_run(r->job, &out);
    C(out == 91);
    return value + 1;
}

static void retained(void)
{
    struct td_context ctx = {2, 0};
    struct td_job job = TD_JOB_INIT;
    int out = 91;
    C(td_submit(&job, td_adapter, &ctx, 5) == TD_OK && ctx.calls == 0);
    C(td_reset(&job) == TD_BUSY &&
      td_submit(&job, td_adapter, &ctx, 9) == TD_STATE);
    C(td_cancel(&job) == TD_OK && job.state == TD_CANCELED && !job.callback &&
      !job.context);
    C(td_run(&job, &out) == TD_STATE && out == 91 && ctx.calls == 0);
    C(td_reset(&job) == TD_OK && td_submit(&job, td_adapter, &ctx, 5) == TD_OK);
    C(td_run(&job, &out) == TD_OK && out == 7 && ctx.calls == 1 &&
      job.state == TD_DONE);
    C(td_run(&job, &out) == TD_STATE && ctx.calls == 1);
    C(td_reset(&job) == TD_OK);
    struct td_reenter r = {&job, TD_OK, TD_OK, TD_OK, 0};
    C(td_submit(&job, reenter, &r, 5) == TD_OK && td_run(&job, &out) == TD_OK &&
      out == 6);
    C(r.calls == 1 && r.cancel == TD_BUSY && r.reset == TD_BUSY &&
      r.reenter == TD_BUSY);
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
