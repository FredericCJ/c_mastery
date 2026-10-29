#include "ipc.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    FILE *f = fopen(argv[1], "r");
    if (!f)
    {
        return 2;
    }
    char line[64];
    unsigned index = 0;
    const unsigned char payload[8] = {0, 1, 2, 3, 128, 254, 255, 0};
    while (fgets(line, sizeof line, f))
    {
        if (!strchr(line, '\n'))
        {
            fclose(f);
            return 2;
        }
        struct ei_request r;
        if (ei_init(&r, payload, 1, 1))
        {
            fclose(f);
            return 3;
        }
        int rc = 0;
        for (unsigned i = 0; line[i] && line[i] != '\n'; ++i)
        {
            if (line[i] == '-')
            {
                continue;
            }
            if (line[i] < '0' || line[i] > '7')
            {
                fclose(f);
                return 2;
            }
            rc = ei_transition(&r, (enum ei_event)(line[i] - '0'));
        }
        printf("%u %d %u %u %u %u %u %u %u %u %d\n", index++, rc,
               (unsigned)r.state, (unsigned)r.owner, (unsigned)r.outcome,
               r.connected, r.effect_done, r.cancel_requested, r.cancel_seen,
               (unsigned)r.result, ei_invariant(&r));
    }
    return fclose(f) ? 2 : 0;
}
