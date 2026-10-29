#include <stdio.h>
int lp_a(void);
int lp_b(void);

int main(void)
{
    if (lp_a() != 5 || lp_b() != 7)
    {
        return 1;
    }
    puts("LP_INLINE values5,7");
    return 0;
}
