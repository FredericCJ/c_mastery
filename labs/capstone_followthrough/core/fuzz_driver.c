#include "service.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 3)
        return 2;
    unsigned chunk = (unsigned)strtoul(argv[1], NULL, 10),
             authorized = (unsigned)strtoul(argv[2], NULL, 10);
    if (!chunk || chunk > 42 || authorized > 1)
        return 2;
    unsigned char bytes[512];
    size_t length = fread(bytes, 1, sizeof bytes, stdin);
    if (!feof(stdin))
        return 2;
    struct cs_service s;
    if (cs_init(&s, 3, 1000) != CS_OK)
        return 3;
    size_t at = 0;
    for (unsigned turn = 0; turn < 4096; ++turn)
    {
        size_t n = length - at;
        if (n > chunk)
            n = chunk;
        size_t used = 0;
        enum cs_status status =
            cs_feed(&s, 0, bytes + at, n, &used, (int)authorized, 0);
        at += used;
        if (status == CS_FRAME)
        {
            if (cs_begin(&s) != CS_OK)
                return 4;
            printf("%u:", s.active.sequence);
            for (size_t i = 6; i < s.active.length - 4; ++i)
                printf("%02x", s.active.bytes[i]);
            putchar('\n');
            if (cs_close(&s, 1) != CS_CLOSED)
                return 5;
            /* New externally quiescent service retains only the parser candidate.
               This driver assesses framing/admission, not cross-message lifecycle. */
            struct cs_input remaining = s.input[0];
            if (cs_init(&s, 3, 1000) != CS_OK)
                return 6;
            s.input[0] = remaining;
        }
        else if (status != CS_MORE && status != CS_REJECT &&
                 status != CS_DENIED)
            return 7;
        if (at == length && status == CS_MORE)
            return 0;
    }
    return 8;
}
