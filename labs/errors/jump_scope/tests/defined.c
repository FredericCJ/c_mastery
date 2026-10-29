#include <setjmp.h>
#include <stdio.h>
static jmp_buf environment;
static unsigned outside;

int main(void)
{
    volatile int changed = 0;
    switch (setjmp(environment))
    {
        case 0:
            changed = 7;
            outside = 11;
            longjmp(environment, 0);
        case 1:
            break;
        default:
            return 1;
    }
    if (changed != 7 || outside != 11)
    {
        return 1;
    }
    if (setjmp(environment) == 0)
    {
        outside = 19;
        longjmp(environment, 2);
    }
    if (outside != 19)
    {
        return 1;
    }
    if (!setjmp(environment))
    {
        outside = 23;
        longjmp(environment, 3);
    }
    if (outside != 23)
    {
        return 1;
    }
    puts(
        "EJD_DEFINED legal-switch-comparison-not volatile-seven external-effects-retained zero-becomes-one");
    return 0;
}
