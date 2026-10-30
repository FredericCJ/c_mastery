/*
 * Stale-token experiment on the foundation handle pool.
 *
 * A handle is acquired and released, then two more values are acquired and
 * released, and finally a fourth value is acquired. The first handle is then
 * presented again.
 *
 * Exit status and marker:
 *   0  the stale handle resolved to the fourth value   OS_WRAP_COLLISION
 *   3  the pool refused the stale handle                OS_WRAP_REFUSED
 *   2  setup failed (an acquire or release was rejected)
 * Which of the first two applies depends on the pool under test.
 */
#include "alias.h"
#include <stdio.h>

enum
{
    CYCLES_AFTER_FIRST = 2, /* acquire/release rounds after the first one */
    FINAL_VALUE = 4
};

int main(void)
{
    struct fd_pool pool;
    struct fd_handle stale = {0};
    struct fd_handle now = {0};
    struct fd_value out = {77, 88};
    fd_pool_init(&pool);
    if (fd_pool_acquire(&pool, (struct fd_value){1, 1}, &stale) != FD_OK ||
        fd_pool_release(&pool, stale) != FD_OK)
    {
        return 2;
    }
    for (int i = 2; i < 2 + CYCLES_AFTER_FIRST; ++i)
    {
        if (fd_pool_acquire(&pool, (struct fd_value){i, 1}, &now) != FD_OK ||
            fd_pool_release(&pool, now) != FD_OK)
        {
            return 2;
        }
    }
    if (fd_pool_acquire(&pool, (struct fd_value){FINAL_VALUE, 1}, &now) !=
        FD_OK)
    {
        return 2;
    }
    if (fd_pool_get(&pool, stale, &out) == FD_OK && out.value == FINAL_VALUE)
    {
        puts("OS_WRAP_COLLISION:stale generation1 names current value4");
        return 0;
    }
    puts("OS_WRAP_REFUSED:retired slot prevents stale-token collision");
    return 3;
}
