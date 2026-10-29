/* Original target string helpers for the retained HTTP implementation.
 * Each source string is live and NUL terminated; each counted span has n bytes.
 * These are not hosted-library links and do not grant safety to invalid spans.
 */
#include <stddef.h>

size_t strlen(const char *s)
{
    size_t n = 0;
    while (s[n])
    {
        ++n;
    }
    return n;
}

char *strcpy(char *d, const char *s)
{
    char *result = d;
    do
    {
        *d++ = *s;
    } while (*s++);
    return result;
}

int strcmp(const char *a, const char *b)
{
    while (*a && *a == *b)
    {
        ++a;
        ++b;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
    {
        unsigned char x = (unsigned char)a[i], y = (unsigned char)b[i];
        if (x != y)
        {
            return x - y;
        }
        if (!x)
        {
            return 0;
        }
    }
    return 0;
}

char *strchr(const char *s, int value)
{
    unsigned char c = (unsigned char)value;
    for (;; ++s)
    {
        if ((unsigned char)*s == c)
        {
            return (char *)s;
        }
        if (!*s)
        {
            return 0;
        }
    }
}

void *memchr(const void *s, int value, size_t n)
{
    const unsigned char *p = s;
    for (size_t i = 0; i < n; ++i)
    {
        if (p[i] == (unsigned char)value)
        {
            return (void *)(p + i);
        }
    }
    return 0;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *x = a, *y = b;
    for (size_t i = 0; i < n; ++i)
    {
        if (x[i] != y[i])
        {
            return x[i] - y[i];
        }
    }
    return 0;
}
