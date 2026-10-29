#include <assert.h>
#include <stdio.h>

int main(void)
{
    unsigned essential = 0;
    assert(++essential);
#ifdef NDEBUG
    if (essential != 0)
        return 1;
    puts("ECM_ASSERT disabled-essential-zero");
#else
    if (essential != 1)
        return 1;
    puts("ECM_ASSERT enabled-essential-one");
#endif
    return 0;
}
