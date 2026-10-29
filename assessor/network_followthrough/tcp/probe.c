#include "test_wire.h"
#include <inttypes.h>

int main(void)
{
    struct nc_rtt r;
    struct nc_stack s;
    char op;
    unsigned a, b;
    uint32_t seq;
    uint64_t now = 80;
    while (scanf(" %c", &op) == 1)
    {
        if (op == 'R')
        {
            if (scanf("%u%u", &a, &b) != 2)
            {
                return 2;
            }
            printf("R %d\n", nc_rtt_init(&r, a, b));
        }
        else if (op == 'S')
        {
            if (scanf("%u%u", &a, &b) != 2)
            {
                return 2;
            }
            int rc = nc_rtt_sample(&r, a, b);
            printf("S %d %u %u %u %u\n", rc, r.srtt, r.variance, r.rto,
                   r.samples);
        }
        else if (op == 'I')
        {
            if (scanf("%u%" SCNu32, &a, &seq) != 2)
            {
                return 2;
            }
            if (nc_establish(&s, a, 20, seq - 1, 256))
            {
                return 3;
            }
            now = 80;
            puts("I 0");
        }
        else if (op == 'D' || op == 'C')
        {
            int rc;
            if (op == 'D')
            {
                if (scanf("%" SCNu32 "%u%u", &seq, &a, &b) != 3 || a > 64 ||
                    b > 255)
                {
                    return 2;
                }
                unsigned char p[104], d[64];
                for (unsigned i = 0; i < a; i++)
                {
                    d[i] = (unsigned char)(b + i);
                }
                size_t n = nc_wire(p, seq, 21, 0x18, 512, d, a, 0);
                rc = nc_receive(&s, p, n, ++now);
            }
            else
            {
                if (scanf("%u", &a) != 1)
                {
                    return 2;
                }
                rc = nc_consume(&s, a);
            }
            printf("%c %d %" PRIu32 " %u %u ", op, rc, s.rcv_nxt, s.rx.count,
                   nw_rx_window(&s.rx));
            for (unsigned i = 0; i < s.rx.count; i++)
            {
                printf("%02x", s.rx.bytes[(s.rx.head + i) % s.rx.capacity]);
            }
            puts("-");
        }
        else
        {
            return 2;
        }
    }
    return 0;
}
