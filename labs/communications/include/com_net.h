#ifndef COM_NET_H
#define COM_NET_H
#include "com_http.h"
#include <stddef.h>
#include <stdint.h>

enum
{
    NET_LISTEN,
    NET_SYN_RECEIVED,
    NET_ESTABLISHED,
    NET_FIN_WAIT_1,
    NET_FIN_WAIT_2,
    NET_CLOSE_WAIT,
    NET_CLOSING,
    NET_LAST_ACK,
    NET_TIME_WAIT
};

struct net_packet
{
    uint8_t bytes[1518];
    size_t length;
};

struct net_stack
{
    struct http_parser http;
    uint8_t state, peer_known, flight, retransmitted, retries, peer_fin,
        fin_sent, interim_pending;
    uint16_t peer_port, peer_window, mss;
    uint32_t snd_una, snd_nxt, rcv_nxt, wl1, wl2, isn;
    uint64_t now, arp_expires, due, sent_at, timewait_until;
    uint32_t srtt, rttvar, rto;
    unsigned rtt_samples, drops, resets, incarnation;
    struct net_packet retained;
    uint8_t response[1600];
    size_t response_length, response_sent, response_acked;
};

uint16_t net_checksum(const void *, size_t, uint32_t initial);
void net_init(struct net_stack *, uint32_t fixture_isn);
/* One owned output packet per call; length=0 means no output. Packet frame
   excludes FCS and preamble supplied by MAC/PHY. Uses fixed peer/local
   addresses. */
int net_receive(struct net_stack *, const void *, size_t, uint64_t,
                struct net_packet *);
int net_tick(struct net_stack *, uint64_t, struct net_packet *);
void net_link_down(struct net_stack *);
/* RFC6298 arithmetic; Karn suppression is caller-visible in transport state. */
void net_rtt(struct net_stack *, uint32_t sample_ms);
#endif
