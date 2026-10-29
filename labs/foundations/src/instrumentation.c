#include <stdint.h>
#include <stdio.h>
static uint64_t calls, returns;

void __attribute__((no_instrument_function))
__cyg_profile_func_enter(void *fn, void *caller)
{
    (void)fn;
    (void)caller;
    calls++;
}

void __attribute__((no_instrument_function))
__cyg_profile_func_exit(void *fn, void *caller)
{
    (void)fn;
    (void)caller;
    returns++;
}

static __attribute__((noinline)) int kernel(int x)
{
    return x * 2;
}

int main(void)
{
    int sum = 0;
    for (int i = 0; i < 4; i++)
        sum += kernel(i);
    printf(
        "{\"sum\":%d,\"entry_hooks\":%llu,\"exit_hooks_before_main_return\":%"
        "llu}\n",
        sum, (unsigned long long)calls, (unsigned long long)returns);
    return sum != 12 || calls != 5 || returns != 4;
}
