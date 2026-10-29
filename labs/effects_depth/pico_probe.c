#include <stdio.h>
#include <string.h>

int main(void)
{
    char integer[32], floating[32];
    int a = snprintf(integer, sizeof integer, "%d", 17),
        b = snprintf(floating, sizeof floating, "%.3f", 0.125);
    if (a != 2 || strcmp(integer, "17"))
    {
        return 1;
    }
    printf("integer=%s floating=%s result=%d\n", integer, floating, b);
    return 0;
}
