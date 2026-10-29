#include "stream.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    FILE *f = fopen(argv[1], "r");
    if (!f)
        return 2;
    char word[128];
    unsigned row = 0;
    while (fgets(word, sizeof word, f))
    {
        struct rf_sequence s;
        if (rf_sequence_init(&s, 1, 7))
            return 3;
        unsigned out[3] = {999, 999, 999}, count = 0, missing = 999;
        int result = 0;
        for (size_t i = 0; i < strcspn(word, "\r\n"); ++i)
        {
            out[0] = out[1] = out[2] = 999;
            count = 0;
            missing = 999;
            unsigned event = (unsigned)(word[i] - '0');
            if (event < 5)
                result = rf_arrive(&s, 1, event + 6, out, &count);
            else if (event == 5)
                result = rf_expire_gap(&s, &missing, out, &count);
            else if (event == 6)
                result = rf_arrive(&s, 2, 0, out, &count);
            else
                return 4;
        }
        printf("%u %d %u %u %u %u %u %u %u %u %u %u %u\n", row++, result,
               s.next, s.pending, s.recent_count, s.recent[0], s.recent[1],
               s.recent[2], count, missing, out[0], out[1], out[2]);
    }
    return fclose(f) ? 2 : 0;
}
