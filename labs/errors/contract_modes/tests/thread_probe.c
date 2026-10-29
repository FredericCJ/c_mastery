#include <threads.h>
#include <stdio.h>

struct er_state
{
    mtx_t mutex;
    int value;
};

static int worker(void *p)
{
    struct er_state *s = p;
    if (mtx_lock(&s->mutex) != thrd_success)
    {
        return 2;
    }
    s->value = 7;
    if (mtx_unlock(&s->mutex) != thrd_success)
    {
        return 3;
    }
    return 7;
}

int main(void)
{
    thrd_t t;
    int r = 0;
    struct er_state s;
    if (mtx_init(&s.mutex, mtx_plain) != thrd_success)
    {
        return 2;
    }
    s.value = 0;
    if (thrd_create(&t, worker, &s) != thrd_success)
    {
        return 2;
    }
    if (thrd_join(t, &r) != thrd_success || r != 7 || s.value != 7)
    {
        return 1;
    }
    mtx_destroy(&s.mutex);
    puts("ECM_C11_THREAD_CONTROL joined-seven");
    return 0;
}
