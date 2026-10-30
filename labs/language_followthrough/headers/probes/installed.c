/* Client of the installed header and static library only. */
#include <headers.h>

int main(void)
{
    const int a[1] = {73};
    int out = 0;
    return hh_read(a, 1, 0, &out) || out != 73 || hh_mix(7) != 22;
}
