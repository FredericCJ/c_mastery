#include "test.h"

static void trace(const unsigned arrivals[3], unsigned before,
                  const unsigned expected_pending[3],
                  const unsigned expected_drop[3],
                  const unsigned expected_service[3])
{
    struct ar_queue q;
    CHECK(ar_queue_init(&q, 4) == AR_OK);
    unsigned id = 0, last = 0;
    for (unsigned tick = 0; tick < 3; ++tick)
    {
        unsigned serviced = 0, dropped = 0;
        struct ar_item item;
        struct ar_eviction e;
        if (before)
            while (serviced < 2 && ar_pop(&q, &item) == AR_OK)
            {
                CHECK(item.id > last);
                last = item.id;
                ++serviced;
            }
        for (unsigned i = 0; i < arrivals[tick]; ++i)
        {
            ++id;
            int r = ar_push(&q, (struct ar_item){id, 0}, 0, &e);
            if (r == AR_FULL)
                ++dropped;
            else
                CHECK(r == AR_OK && !e.valid);
        }
        if (!before)
            while (serviced < 2 && ar_pop(&q, &item) == AR_OK)
            {
                CHECK(item.id > last);
                last = item.id;
                ++serviced;
            }
        CHECK(q.length == expected_pending[tick] &&
              dropped == expected_drop[tick] &&
              serviced == expected_service[tick]);
    }
}

int main(void)
{
    struct ar_queue q;
    struct ar_eviction e = {9, {9, 9}};
    struct ar_item item;
    CHECK(ar_queue_init(&q, 2) == AR_OK);
    CHECK(ar_push(&q, (struct ar_item){1, 1}, 0, &e) == AR_OK && !e.valid);
    CHECK(ar_push(&q, (struct ar_item){2, 1}, 0, &e) == AR_OK);
    CHECK(ar_push(&q, (struct ar_item){3, 1}, 0, &e) == AR_FULL);
    CHECK(ar_pop(&q, &item) == AR_OK && item.id == 1);
    CHECK(ar_pop(&q, &item) == AR_OK && item.id == 2);
    CHECK(ar_pop(&q, &item) == AR_EMPTY);
    CHECK(ar_queue_init(&q, 2) == AR_OK);
    CHECK(ar_push(&q, (struct ar_item){1, 1}, 1, &e) == AR_OK);
    CHECK(ar_push(&q, (struct ar_item){2, 1}, 1, &e) == AR_OK);
    CHECK(ar_push(&q, (struct ar_item){3, 1}, 1, &e) == AR_OK && e.valid &&
          e.item.id == 1);
    CHECK(ar_pop(&q, &item) == AR_OK && item.id == 2);
    CHECK(ar_pop(&q, &item) == AR_OK && item.id == 3);
    CHECK(ar_push(&q, (struct ar_item){4, 1}, 2, &e) == AR_OK);
    CHECK(ar_push(&q, (struct ar_item){5, 3}, 2, &e) == AR_OK);
    CHECK(ar_push(&q, (struct ar_item){6, 2}, 2, &e) == AR_OK && e.valid &&
          e.item.id == 4);
    CHECK(ar_push(&q, (struct ar_item){7, 2}, 2, &e) == AR_FULL);
    CHECK(ar_pop(&q, &item) == AR_OK && item.id == 5);
    CHECK(ar_pop(&q, &item) == AR_OK && item.id == 6);
    CHECK(ar_queue_init(&q, 0) == AR_OK &&
          ar_push(&q, (struct ar_item){1, 1}, 0, &e) == AR_FULL);
#if AR_CAP == 4
    trace((unsigned[]){3, 1, 4}, 0, (unsigned[]){1, 0, 2},
          (unsigned[]){0, 0, 0}, (unsigned[]){2, 2, 2});
    trace((unsigned[]){5, 0, 5}, 0, (unsigned[]){2, 0, 2},
          (unsigned[]){1, 0, 1}, (unsigned[]){2, 2, 2});
    trace((unsigned[]){3, 1, 4}, 1, (unsigned[]){3, 2, 4},
          (unsigned[]){0, 0, 0}, (unsigned[]){0, 2, 2});
    trace((unsigned[]){5, 0, 5}, 1, (unsigned[]){4, 2, 4},
          (unsigned[]){1, 0, 1}, (unsigned[]){0, 2, 2});
#else
    (void)trace;
#endif
    puts("PASS queue identity,ownership,priority,enqueue/service order");
}
