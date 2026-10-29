#include "congestion.h"
#include <inttypes.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    (void)argv;
    if (argc > 1)
    {
        printf("NR_LAYOUT state=%zu alignment=%zu\n", sizeof(struct nr_control),
               _Alignof(struct nr_control));
        return 0;
    }
    char op;
    unsigned a, b, c;
    struct nr_control s;
    uint32_t seq;
    while (scanf(" %c%u%u%u", &op, &a, &b, &c) == 4)
    {
        if (op == 'P')
        {
            if (!a || a > 12 || nr_init(&s, a, 0xfffffff8, 65535) ||
                nr_send(&s, a, 1, 0, &seq))
            {
                return 2;
            }
            for (unsigned i = 1; i <= a; i++)
            {
                if (i == a || (b & (1u << (i - 1))))
                {
                    if (nr_ack(&s, 0xfffffff8 + i, 65535, 0, 16, i))
                    {
                        return 3;
                    }
                }
            }
        }
        else if (op == 'T')
        {
            if (!a || a > 128 || !b || b > 16 ||
                nr_init(&s, b, 0xfffffff0, 65535))
            {
                return 4;
            }
            s.nxt += a;
            s.cwnd = a * 4 + 1024;
            if (nr_timeout(&s, 1000))
            {
                return 5;
            }
            uint32_t threshold = s.ssthresh;
            s.cwnd = a + b;
            if (nr_send(&s, b, 1, 1001, &seq) || nr_timeout(&s, 3000))
            {
                return 6;
            }
            printf("T %u %u %u\n", threshold, s.ssthresh, s.cwnd);
            continue;
        }
        else if (op == 'Q')
        {
            if (!a || a > 32 || b < 2 || b > 8 || !c ||
                nr_init(&s, a, 0xfffffff0, 65535))
            {
                return 7;
            }
            s.cwnd = s.ssthresh = a * b;
            for (unsigned i = 0; i < b; i++)
            {
                if (nr_send(&s, a, 1, 0, &seq))
                {
                    return 8;
                }
            }
            unsigned acknowledged = 0;
            while (acknowledged < a * b)
            {
                unsigned n = a * b - acknowledged;
                if (n > c)
                {
                    n = c;
                }
                acknowledged += n;
                if (nr_ack(&s, 0xfffffff0 + acknowledged, 65535, 0, 16,
                           acknowledged))
                {
                    return 9;
                }
            }
        }
        else if (op == 'L')
        {
            if (!a || a > 16 || b < 4 || b > 8 || c > 2 * a ||
                nr_init(&s, a, 0xfffffff0, 65535) || nr_policy(&s, 1))
            {
                return 11;
            }
            s.cwnd = a * b;
            for (unsigned i = 0; i < b; i++)
            {
                if (nr_send(&s, a, 1, 0, &seq))
                {
                    return 12;
                }
            }
            for (unsigned i = 1; i <= 2; i++)
            {
                if (nr_ack(&s, s.una, 65535, 0, 16, i) != NR_WAIT ||
                    nr_send(&s, a, 1, i, &seq))
                {
                    return 13;
                }
            }
            if (nr_ack(&s, s.una, 65535, 0, 16, 3) != NR_FAST_RETRANSMIT)
            {
                return 14;
            }
            uint32_t threshold = s.ssthresh;
            if (nr_ack(&s, s.una + a * b + c, 65535, 0, 16, 4))
            {
                return 15;
            }
            printf("L %u %u %u\n", threshold,
                   s.limited_length[0] + s.limited_length[1], nr_flight(&s));
            continue;
        }
        else
        {
            return 10;
        }
        printf("%c %u %" PRIu32 " %u\n", op, s.cwnd, s.una, nr_flight(&s));
    }
    return 0;
}
