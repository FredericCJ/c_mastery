/* Original finite lost-update teaching target. Preserve both policies. */
#include <pthread.h>
#include <stdatomic.h>
#include <stdio.h>
#include <string.h>
#ifndef TM_SCHEDULE_REPAIR
#define TM_SCHEDULE_REPAIR 1
#endif
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t changed = PTHREAD_COND_INITIALIZER;
static _Atomic unsigned value;
static const char *schedule;
static size_t next_event;
static int failure;

static void *worker(void *arg)
{
    char id = *(char *)arg;
    unsigned snapshot = 0;
    for (unsigned phase = 0; phase < 2; phase++)
    {
        char expected = phase ? (char)(id - 'a' + 'A') : id;
        if (pthread_mutex_lock(&lock))
        {
            return &failure;
        }
        for (;;)
        {
            while (schedule[next_event] == '.')
            {
                next_event++;
            }
            if (schedule[next_event] == expected)
            {
                break;
            }
            if (pthread_cond_wait(&changed, &lock))
            {
                (void)pthread_mutex_unlock(&lock);
                return &failure;
            }
        }
        if (!phase)
        {
            snapshot = atomic_load(&value);
        }
        else if (TM_SCHEDULE_REPAIR)
        {
            (void)atomic_fetch_add(&value, 1);
        }
        else
        {
            atomic_store(&value, snapshot + 1);
        }
        next_event++;
        (void)pthread_cond_broadcast(&changed);
        if (pthread_mutex_unlock(&lock))
        {
            return &failure;
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    if (argc != 2 || strlen(argv[1]) > 32)
    {
        return 2;
    }
    static const char letters[] = "aAbB";
    unsigned counts[4] = {0};
    int positions[4] = {0};
    for (size_t i = 0; argv[1][i]; i++)
    {
        const char *at = strchr(letters, argv[1][i]);
        if (argv[1][i] == '.')
        {
            continue;
        }
        if (!at)
        {
            return 2;
        }
        unsigned k = (unsigned)(at - letters);
        counts[k]++;
        positions[k] = (int)i;
    }
    for (unsigned i = 0; i < 4; i++)
    {
        if (counts[i] != 1)
        {
            return 2;
        }
    }
    if (positions[0] > positions[1] || positions[2] > positions[3])
    {
        return 2;
    }
    schedule = argv[1];
    pthread_t threads[2];
    char ids[2] = {'a', 'b'};
    void *r;
    for (unsigned i = 0; i < 2; i++)
    {
        if (pthread_create(&threads[i], NULL, worker, &ids[i]))
        {
            return 2;
        }
    }
    for (unsigned i = 0; i < 2; i++)
    {
        if (pthread_join(threads[i], &r) || r)
        {
            return 2;
        }
    }
    unsigned result = atomic_load(&value);
    printf("SCHEDULE %s %u\n", result == 2 ? "none" : "lost-update", result);
    if (pthread_cond_destroy(&changed) || pthread_mutex_destroy(&lock))
    {
        return 2;
    }
    return 0;
}
