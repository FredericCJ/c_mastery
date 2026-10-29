#include "lease.h"
#include <stdio.h>

int main(void)
{
    /* All 9^4 event words, not random examples. The Python oracle uses a
     * symbolic transition table and compares every return and owned field. */
    for (unsigned word = 0; word < 6561; ++word)
    {
        struct xd_lease s = {0};
        struct xd_fence f = {0, 1, 1, 1};
        struct xd_ticket t;
        if (xd_recover(&s, &f) || xd_submit(&s, 32, &t))
        {
            return 2;
        }
        unsigned x = word;
        for (unsigned step = 0; step < 4; ++step, x /= 9)
        {
            unsigned event = x % 9;
            int status = XD_OK;
            struct xd_proof p = {t, 0, 0, 1, 32, 32};
            struct xd_result result = {0};
            switch (event)
            {
                case 0:
                    status = xd_cancel(&s, t);
                    break;
                case 1:
                    status = xd_observe(&s, t, XD_COMPLETE, &p);
                    break;
                case 2:
                    p.complete = p.possible = 8;
                    status = xd_observe(&s, t, XD_STOPPED, &p);
                    break;
                case 3:
                    p.writes_pending = 1;
                    status = xd_observe(&s, t, XD_COMPLETE, &p);
                    break;
                case 4:
                    status = xd_reclaim(&s, t, &result);
                    break;
                case 5:
                    p.ticket.generation = 2;
                    status = xd_observe(&s, p.ticket, XD_COMPLETE, &p);
                    break;
                case 6:
                    xd_quarantine(&s);
                    break;
                case 7:
                    f = (struct xd_fence){s.epoch, s.epoch + 1, 1, 0};
                    status = xd_recover(&s, &f);
                    break;
                case 8:
                    f = (struct xd_fence){s.epoch, s.epoch + 1, 1, 1};
                    status = xd_recover(&s, &f);
                    break;
            }
            printf("%u %u %u %d %u %u %u %u %u %u %u %u\n", word, step, event,
                   status, s.epoch, s.generation, s.phase, s.terminal,
                   s.terminal_count, s.complete, s.possible, s.quiescent);
        }
    }
}
