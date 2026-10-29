#include <stdio.h>

/* Original, intentionally flawed local queue: no expired-pointer access. The
 * live sender changes a byte, exposing failure to take the promised snapshot.
 */
int main(void)
{
    unsigned char sender[4] = {3, 0, 9, 7};

    struct
    {
        const unsigned char *bytes;
        unsigned length;
    } bad = {sender, 4};

    sender[0] = 88;
    if (bad.length != 4 || bad.bytes[0] != 3)
    {
        fprintf(stderr,
                "CL_CHECK:pointer-copy returned changed live sender byte\n");
        return 1;
    }
    return 0;
}
