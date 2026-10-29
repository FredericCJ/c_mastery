#include "congestion.h"
#include <stdio.h>
#define CHECK(x)                  \
    do                            \
    {                             \
        if (!(x))                 \
        {                         \
            puts("NR_CHECK " #x); \
            return 1;             \
        }                         \
    } while (0)

int main(void)
{
    struct nr_control s;
    uint32_t seq = 123;
    CHECK(nr_init(&s, 100, 0xfffffff0, 50) == NR_OK);
    CHECK(nr_usable(&s) == 50);
    CHECK(nr_send(&s, 51, 1, 0, &seq) == NR_BUSY && seq == 123);
    CHECK(nr_send(&s, 50, 0, 0, &seq) == NR_WAIT && nr_flight(&s) == 0);
    CHECK(nr_send(&s, 50, 1, 0, &seq) == NR_OK && seq == 0xfffffff0 &&
          s.nxt == 34);
    CHECK(nr_ack(&s, 35, 50, 0, 16, 1) == NR_INVALID && nr_flight(&s) == 50);
    for (unsigned i = 1; i <= 50; i++)
    {
        CHECK(nr_ack(&s, 0xfffffff0 + i, 50, 0, 16, i) == NR_OK);
    }
    CHECK(s.cwnd == 150 &&
          s.una == 34); /* ACK division admits only50 growth, not5000. */
    CHECK(nr_init(&s, 100, 0, 10000) == NR_OK);
    for (unsigned round = 0; round < 3; round++)
    {
        unsigned target = s.cwnd;
        while (nr_flight(&s) < target)
        {
            CHECK(nr_send(&s, 100, 1, round * 10, &seq) == NR_OK);
        }
        while (nr_flight(&s))
        {
            CHECK(nr_ack(&s, s.una + 100, 10000, 0, 16, round * 10 + 1) ==
                  NR_OK);
        }
    }
    CHECK(s.cwnd ==
          800); /* cwnd8MSS, then rwnd permits only4MSS actual flight. */
    CHECK(nr_ack(&s, s.una, 400, 0, 16, 31) == NR_WAIT);
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(nr_send(&s, 100, 1, 32, &seq) == NR_OK);
    }
    CHECK(nr_ack(&s, s.una, 401, 0, 16, 33) == NR_WAIT && s.duplicates == 0);
    CHECK(nr_ack(&s, s.una, 401, 1, 16, 34) == NR_WAIT && s.duplicates == 0);
    CHECK(nr_ack(&s, s.una, 401, 0, 16, 35) == NR_WAIT);
    CHECK(nr_ack(&s, s.una, 401, 0, 16, 36) == NR_WAIT);
    CHECK(nr_ack(&s, s.una, 401, 0, 16, 37) == NR_FAST_RETRANSMIT);
    CHECK(s.ssthresh == 200 && s.cwnd == 500 && s.recovery);
    CHECK(nr_ack(&s, s.una, 401, 0, 16, 38) == NR_WAIT);
    CHECK(s.cwnd == 600);
    for (unsigned i = 0; i < 20; i++)
    {
        CHECK(nr_ack(&s, s.una, 401, 0, 16, 39 + i) == NR_WAIT);
    }
    CHECK(s.cwnd == 600);
    /* Window updates reset duplicate classification, not the recovery episode's
     * inflation budget. Alternating them must not replenish that budget. */
    for (unsigned i = 0; i < 20; i++)
    {
        unsigned window = i & 1 ? 400 : 401;
        CHECK(nr_ack(&s, s.una, window, 0, 16, 59) == NR_WAIT);
        CHECK(nr_ack(&s, s.una, window, 0, 16, 59) == NR_WAIT);
    }
    CHECK(s.cwnd == 600 && !s.quota);
    CHECK(nr_ack(&s, s.nxt, 10000, 0, 16, 60) == NR_OK && s.cwnd == 200 &&
          !s.recovery);
    for (unsigned i = 0; i < 2; i++)
    {
        CHECK(nr_send(&s, 100, 1, 61, &seq) == NR_OK);
    }
    CHECK(nr_ack(&s, s.una + 100, 10000, 0, 16, 62) == NR_OK && s.cwnd == 200 &&
          s.credit == 100);
    CHECK(nr_ack(&s, s.una + 100, 10000, 0, 16, 63) == NR_OK && s.cwnd == 300 &&
          s.credit == 0);
    CHECK(nr_restart(&s, 1061, 1000) == NR_OK && s.cwnd == 300);
    CHECK(nr_restart(&s, 1062, 1000) == NR_OK && s.cwnd == 100);
    /* Same lost position's repeated timeout preserves threshold. An externally
     * constructed valid flight8MSS ledger tests the mechanism independent of ACK
     * schedule; simulator tests construct their state by actual sends. */
    CHECK(nr_init(&s, 100, 0, 10000) == NR_OK);
    s.cwnd = 800;
    s.nxt = 800;
    CHECK(nr_timeout(&s, 1000) == NR_OK && s.ssthresh == 400 && s.cwnd == 100);
    for (unsigned i = 0; i < 6; i++)
    {
        int rc = nr_ack(&s, s.una, 10000, 0, 16, 1001);
        CHECK(rc == (i == 2 ? NR_FAST_RETRANSMIT : NR_WAIT));
    }
    CHECK(s.cwnd == 1000);
    CHECK(nr_send(&s, 100, 1, 1002, &seq) == NR_OK);
    CHECK(nr_send(&s, 100, 1, 1002, &seq) == NR_OK);
    CHECK(nr_timeout(&s, 3000) == NR_OK && s.ssthresh == 400);
    CHECK(nr_ack(&s, 100, 10000, 0, 16, 3100) == NR_OK);
    CHECK(nr_timeout(&s, 4000) == NR_OK && s.ssthresh == 450);
    CHECK(nr_init(&s, 100, 0xfffffff0, 1600) == NR_OK);
    CHECK(nr_policy(&s, 1) == NR_OK);
    s.cwnd = 800;
    for (unsigned i = 0; i < 8; i++)
    {
        CHECK(nr_send(&s, 100, 1, 0, &seq) == NR_OK);
    }
    CHECK(nr_policy(&s, 0) == NR_BUSY);
    CHECK(nr_ack(&s, s.una, 1600, 0, 16, 1) == NR_WAIT && s.cwnd == 800);
    CHECK(nr_send(&s, 100, 0, 1, &seq) == NR_WAIT && s.quota == 1);
    CHECK(nr_send(&s, 100, 1, 1, &seq) == NR_OK && s.cwnd == 800);
    CHECK(nr_send(&s, 1, 1, 1, &seq) == NR_BUSY);
    CHECK(nr_ack(&s, s.una, 1600, 0, 16, 2) == NR_WAIT);
    CHECK(nr_send(&s, 100, 1, 2, &seq) == NR_OK && nr_flight(&s) == 1000);
    CHECK(nr_ack(&s, s.una, 1600, 0, 16, 3) == NR_FAST_RETRANSMIT);
    CHECK(s.ssthresh == 400 &&
          s.cwnd == 700); /* exclude200 limited-transmit bytes */
    CHECK(nr_ack(&s, s.una + 900, 1600, 0, 16, 4) == NR_OK);
    CHECK(s.limited_length[0] == 0 && s.limited_length[1] == 100);
    CHECK(nr_ack(&s, s.nxt, 1600, 0, 16, 5) == NR_OK);
    CHECK(!s.limited_length[0] && !s.limited_length[1] &&
          nr_policy(&s, 0) == NR_OK);
    puts(
        "NR_PASS flight_window_ACK_division_fast_recovery_timeout_idle_limited");
    return 0;
}
