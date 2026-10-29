/* Reuse the actual public synthetic peer, rather than claiming a socket is the
 * learner TCP implementation. NB_TARGET only renames its public test entry and
 * places its explicitly owned DMA storage in .eth; no hardware is inferred. */
#define NB_TARGET
#include "network_followthrough/http/tests.c"
#undef main
#undef NB_TARGET

int eh_stack(void)
{
    CHECK(!nh_public_tests());
    /* A fresh GET still uses actual EnRing/ARP/IP/TCP/HTTP. Seven-byte peer
     * writes, MSS17 and receive capacity16 force independently chunked service
     * handoffs. */
    request_length = 0;
    text_request("GET /health HTTP/1.1\r\nHost: mcu.test\r\n\r\n");
    CHECK(!start(17, 9, 16, 0));
    CHECK(!exchange(7, 0));
    static const char want[] =
        "HTTP/1.1 200 OK\r\nContent-Type: text/plain; "
        "charset=utf-8\r\nContent-Length: 3\r\nConnection: close\r\n\r\nok\n";
    CHECK(captured == sizeof want - 1 &&
          !memcmp(capture, want, sizeof want - 1));
    CHECK(net.rx_frames == net.rx_released && !model.violation &&
          net.tcp.state == NC_TIME_WAIT);
    /* Application DONE is not completion of TIME_WAIT. Drive actual timer to its
     * deadline; storage stays alive, and the retired application is not
     * restarted. */
    uint64_t due = net.tcp.timewait_until;
    CHECK(due >= time_ms);
    CHECK(nb_step(&net, due) == NB_OK && net.tcp.state == NC_CLOSED);
    puts("EH_STACK_PASS "
         "actual_raw_GET_frames_stream_time_events_DMA_ACK_TIME_WAIT");
    return 0;
}
