/*
 * Formats an integer and a floating-point value with snprintf and prints both.
 * Built against two picolibc configurations, so the output is a record of
 * what each configuration supports. Exit status 1 when the integer text is
 * not "17".
 */
#include <stdio.h>
#include <string.h>

#define TEXT_BYTES 32u

int main(void)
{
    char integer_text[TEXT_BYTES];
    char floating_text[TEXT_BYTES];
    int integer_length = snprintf(integer_text, sizeof integer_text, "%d", 17);
    int floating_length =
        snprintf(floating_text, sizeof floating_text, "%.3f", 0.125);
    if (integer_length != 2 || strcmp(integer_text, "17"))
    {
        return 1;
    }
    printf("integer=%s floating=%s result=%d\n", integer_text, floating_text,
           floating_length);
    return 0;
}
