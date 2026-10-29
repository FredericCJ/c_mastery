#include "picolib.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>

/* Exit codes are public named contract checks because stdout is the target.
   11=scaffold,12=output,13=overflow,14=input,15=nonseekable,16=reset. */
int main(void)
{
    unsigned char data[4];
    if (lp_reset())
    {
        return 11;
    }
    if (printf("xy%d", 17) != 4 || lp_output(data) != 4 ||
        memcmp(data, "xy17", 4))
    {
        return 12;
    }
    if (fputc('Z', stdout) != EOF)
    {
        return 13;
    }
    if (getchar() != 'a' || getchar() != 'b' || getchar() != 'c' ||
        getchar() != EOF)
    {
        return 14;
    }
    errno = 0;
    if (fseek(stdout, 0, SEEK_SET) != -1 || errno != ESPIPE)
    {
        return 15;
    }
    if (lp_reset() || lp_output(data) != 0 || getchar() != 'a')
    {
        return 16;
    }
    return 0;
}
