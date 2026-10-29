#include <unistd.h>
#include <errno.h>

/* Original local endless-driver specimen: only its private inherited pipe.
 * The supervisor retains the writer without writing, then kills/reaps child. */
int main(void)
{
    char c;
    for (;;)
    {
        ssize_t n = read(0, &c, 1);
        if (n > 0)
            return 4;
        if (n == 0)
            return 5;
        if (errno != EINTR)
            return 6;
    }
}
