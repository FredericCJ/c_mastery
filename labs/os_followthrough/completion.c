#include "memory.h"
#include <stdio.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            fprintf(stderr, "OF_COMPLETION:%d:%s\n", __LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

static int finish(struct mw_async *a, struct mw_ticket t)
{
    CHECK(mw_terminal(a, t) == MW_OK && mw_callback_begin(a, t, 0) == MW_OK &&
          mw_callback_end(a, t) == MW_OK && mw_reclaim(a, t) == MW_OK);
    return 0;
}

int main(void)
{
    struct mw_async a;
    mw_async_init(&a);
    struct mw_ticket t;
    for (unsigned i = 1; i < 4; i++)
    {
        CHECK(mw_submit(&a, i, &t) == MW_OK && t.generation == i &&
              mw_submit_return(&a, t) == MW_OK);
        CHECK(!finish(&a, t));
    }
    CHECK(mw_submit(&a, 7, &t) == MW_OK && t.generation == 4);
    struct mw_ticket stale = t;
    stale.generation = 3;
    stale.request = 12;
    CHECK(mw_terminal(&a, stale) == MW_STALE);
    CHECK(mw_cancel(&a, t) == MW_OK && mw_reclaim(&a, t) == MW_BUSY);
    CHECK(mw_terminal(&a, t) == MW_OK);
    struct mw_ticket duplicate = t;
    duplicate.request = 8;
    CHECK(mw_terminal(&a, duplicate) == MW_STALE &&
          mw_terminal(&a, t) == MW_STALE);
    CHECK(mw_reclaim(&a, t) == MW_BUSY &&
          mw_callback_begin(&a, t, 0) == MW_OK &&
          mw_reclaim(&a, t) == MW_BUSY && mw_callback_end(&a, t) == MW_OK &&
          mw_reclaim(&a, t) == MW_BUSY && mw_submit_return(&a, t) == MW_OK &&
          mw_reclaim(&a, t) == MW_OK);
    CHECK(a.results == 4);
    struct mw_ticket first, second;
    CHECK(mw_submit(&a, 21, &first) == MW_OK &&
          mw_submit(&a, 22, &second) == MW_OK &&
          mw_submit(&a, 23, &t) == MW_BUSY);
    CHECK(mw_submit_return(&a, first) == MW_OK &&
          mw_submit_return(&a, second) == MW_OK);
    unsigned shutting_down = 1;
    CHECK(shutting_down && mw_cancel(&a, first) == MW_OK &&
          mw_cancel(&a, second) == MW_OK);
    CHECK(!finish(&a, second));
    CHECK(a.slots[first.slot].started && mw_reclaim(&a, first) == MW_BUSY);
    CHECK(!finish(&a, first));
    CHECK(!a.callbacks && !a.scratch_mask && !a.slots[0].started &&
          !a.slots[1].started);
    CHECK(mw_terminal(&a, first) == MW_STALE &&
          mw_terminal(&a, second) == MW_STALE);
    a.slots[0].generation = UINT32_MAX;
    a.slots[1].retired = 1;
    CHECK(mw_submit(&a, 31, &t) == MW_OK && mw_submit_return(&a, t) == MW_OK &&
          !finish(&a, t) && a.slots[0].retired &&
          mw_submit(&a, 32, &t) == MW_BUSY);
    puts("OF_PASS:completion:gen4-only7:cancel-retains:pending-active-submit-"
         "gates:out-of-order:shutdown-quiescent:max-retired");
    return 0;
}
