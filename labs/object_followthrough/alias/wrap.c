#include "alias.h"
#include <stdio.h>

int main(void)
{
    struct fd_pool p;
    struct fd_handle stale = {0}, now = {0};
    struct fd_value out = {77, 88};
    fd_pool_init(&p);
    if (fd_pool_acquire(&p, (struct fd_value){1, 1}, &stale) != FD_OK ||
        fd_pool_release(&p, stale) != FD_OK)
        return 2;
    for (int i = 2; i <= 3; ++i)
        if (fd_pool_acquire(&p, (struct fd_value){i, 1}, &now) != FD_OK ||
            fd_pool_release(&p, now) != FD_OK)
            return 2;
    if (fd_pool_acquire(&p, (struct fd_value){4, 1}, &now) != FD_OK)
        return 2;
    if (fd_pool_get(&p, stale, &out) == FD_OK && out.value == 4)
    {
        puts("OS_WRAP_COLLISION:stale generation1 names current value4");
        return 0;
    }
    puts("OS_WRAP_REFUSED:retired slot prevents stale-token collision");
    return 3;
}
