#include "handoff.h"
#include <assert.h>
#include <pthread.h>
static struct xi_queue q;
static unsigned published, consumed;

static void *producer(void *unused)
{
    (void)unused;
    for (unsigned i = 0; i < 3; ++i)
    {
        struct xi_record r;
        r.sequence = i;
        r.timestamp = i * 3;
        r.original_length = 2;
        r.copied = 2;
        r.flags = 0;
        for (unsigned j = 0; j < 8; ++j)
        {
            r.bytes[j] = j == 0   ? (unsigned char)i
                         : j == 1 ? (unsigned char)(i + 7)
                                  : 0;
        }
        int rc = xi_push(&q, &r);
        assert(rc == XI_OK || rc == XI_FULL);
        published += rc == XI_OK;
    }
    return 0;
}

static void *consumer(void *unused)
{
    (void)unused;
    unsigned seen = 0, prior = 0;
    for (unsigned i = 0; i < 3; ++i)
    {
        struct xi_record r;
        int rc = xi_pop(&q, &r);
        assert(rc == XI_OK || rc == XI_EMPTY);
        if (rc == XI_OK)
        {
            assert(r.sequence < 3 && r.timestamp == 3 * r.sequence &&
                   r.copied == 2 && r.bytes[0] == r.sequence &&
                   r.bytes[1] == r.sequence + 7);
            assert(!seen || r.sequence > prior);
            prior = r.sequence;
            ++seen;
            ++consumed;
        }
    }
    return 0;
}

int main(void)
{
    pthread_t p, c;
    assert(xi_init(&q, 2) == XI_OK);
    pthread_create(&p, 0, producer, 0);
    pthread_create(&c, 0, consumer, 0);
    pthread_join(p, 0);
    pthread_join(c, 0);
    struct xi_record r;
    while (xi_pop(&q, &r) == XI_OK)
    {
        assert(r.sequence < 3 && r.timestamp == 3 * r.sequence &&
               r.copied == 2 && r.bytes[0] == r.sequence &&
               r.bytes[1] == r.sequence + 7);
        ++consumed;
    }
    assert(published > 0 && published <= 3 && consumed == published);
    return 0;
}
