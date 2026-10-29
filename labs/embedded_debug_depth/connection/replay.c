#include "connection.h"
#include <stdio.h>

int main(void)
{
    for (unsigned word = 0; word < 4096; ++word)
    {
        struct dc_manifest m = {511, 1, 2, 3, 4, 5, 0x413, 6, 7, 8, 0, 0};
        struct dc_session s;
        enum dc_status status = dc_init(&s, &m);
        unsigned trace = word;
        if (status != DC_OK)
            return 1;
        for (unsigned step = 0; step < 4; ++step, trace /= 8)
        {
            switch (trace % 8)
            {
                case 0:
                    status = dc_identify(&s, 0x413);
                    break;
                case 1:
                    status = dc_halt(&s, 1);
                    break;
                case 2:
                    status = dc_reset(&s, 1);
                    break;
                case 3:
                    status = dc_program_authorized(&s);
                    break;
                case 4:
                    status = dc_identify(&s, 0x419);
                    break;
                case 5:
                    ++m.board;
                    status = dc_reconfigure(&s, &m);
                    break;
                case 6:
                    m.power = 0;
                    m.known &= ~DC_POWER;
                    status = dc_reconfigure(&s, &m);
                    break;
                default:
                    m.power = 7;
                    m.known |= DC_POWER;
                    status = dc_reconfigure(&s, &m);
                    break;
            }
        }
        printf("%u %u %u %u %u %u\n", word, (unsigned)status, s.phase,
               s.observed_target, s.manifest.known, s.manifest.board);
    }
    return 0;
}
