#include "service.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2 || strlen(argv[1]) > 64)
    {
        return 2;
    }
    struct cs_service s;
    if (cs_init(&s, 3, 100) != CS_OK)
    {
        return 3;
    }
    struct cp_message m = {1, 0, {0}};
    const unsigned char *borrow = NULL;
    size_t n = 0;
    unsigned generation = 0;
    int external_borrow = 0;
    for (const char *p = argv[1]; *p; ++p)
    {
        unsigned before = s.accepted;
        switch (*p)
        {
            case 'A':
                (void)cs_offer(&s, &m, 1, 0, 0);
                break;
            case 'U':
                (void)cs_offer(&s, &m, 0, 0, 0);
                if (s.accepted != before)
                {
                    puts("CAP_SECURITY:authority");
                    return 41;
                }
                break;
            case 'B':
                (void)cs_begin(&s);
                break;
            case 'L':
                if (cs_lease(&s, &borrow, &n, &generation) == CS_OK)
                {
                    external_borrow = 1;
                }
                break;
            case 'X':
                (void)cs_close(&s, 1);
                break;
            case 'Q':
                (void)cs_complete(&s, generation, 0, 0, NULL, NULL);
                break;
            case 'C':
            {
                enum cs_status status =
                    cs_complete(&s, generation, 1, 0, NULL, NULL);
                if (status != CS_STALE && status != CS_INVALID)
                {
                    external_borrow = 0;
                }
                break;
            }
            default:
                return 2;
        }
        if (external_borrow && !s.has_active)
        {
            puts("CAP_SECURITY:retained");
            return 42;
        }
        if (!cs_conserved(&s))
        {
            return 4;
        }
    }
    puts("CAP_SECURITY:bounded-safe");
    return 0;
}
