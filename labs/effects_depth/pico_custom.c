/*
 * picolibc with a user-supplied console: the three standard streams are one
 * FDEV stream whose put callback appends to an in-memory buffer. Exit status
 * 1 when the printf check fails, 2 when the fseek check fails.
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define CONSOLE_BYTES 64u

static char output[CONSOLE_BYTES];
static unsigned used;

/* Appends one character; EOF once the buffer is full. */
static int put(char c, FILE *stream)
{
    (void)stream;
    if (used >= sizeof output)
    {
        return EOF;
    }
    output[used++] = c;
    return (unsigned char)c;
}

/* The console has no input. */
static int get(FILE *stream)
{
    (void)stream;
    return EOF;
}

static FILE console = FDEV_SETUP_STREAM(put, get, NULL, _FDEV_SETUP_RW);
FILE *const stdin = &console;
FILE *const stdout = &console;
FILE *const stderr = &console;

int main(void)
{
    static const char expected[] = "value=17";
    const int expected_length = (int)(sizeof expected - 1);
    if (printf("value=%d", 17) != expected_length ||
        used != (unsigned)expected_length ||
        memcmp(output, expected, sizeof expected - 1))
    {
        return 1;
    }
    errno = 0;
    if (fseek(stdout, 0, SEEK_SET) != -1 || errno != ESPIPE)
    {
        return 2;
    }
    return 0;
}
