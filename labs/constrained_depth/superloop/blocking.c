/* Original, intentionally nonyielding callback. A host observer terminates only
 * this disposable child after the entered marker; no real device or blocking
 * system API is involved. This is a failure-of-progress experiment, not a
 * positive scheduler implementation or a semantic mutation pass. */
#include <stdio.h>
static volatile unsigned never_released;

int main(void)
{
    puts("SC_BLOCK_ENTER:nonyielding-driver");
    fflush(stdout);
    while (!never_released)
    {
    }
    puts("SC_BLOCK_RETURN:unexpected");
    return 0;
}
