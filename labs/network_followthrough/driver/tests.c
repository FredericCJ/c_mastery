#include "../tcp/test_wire.h"
#include "driver.h"
#include "ethernet_followthrough/phy/model.h"
#include "ethernet_followthrough/ring/model.h"
#undef CHECK
#define CHECK(x)                  \
    do                            \
    {                             \
        if (!(x))                 \
        {                         \
            puts("NB_CHECK " #x); \
            return 1;             \
        }                         \
    } while (0)
#ifdef NB_TARGET
struct en_memory nb_memory __attribute__((section(".eth"), aligned(4)));
#else
struct en_memory nb_memory;
#endif
#define memory nb_memory
static struct en_model model;
static struct ep_model phy_model;
static struct ep_phy phy;
static struct en_ring ring;
static struct nb_net net;
static const unsigned char local_mac[6] = {2, 0, 0, 0, 0, 2},
                           peer_mac[6] = {2, 0, 0, 0, 0, 1};
static const struct na_config config = {{2, 0, 0, 0, 0, 2},
                                        {192, 0, 2, 2},
                                        {192, 0, 2, 1}};

static int boot(void)
{
    memset(&phy, 0, sizeof phy);
    memset(&ring, 0, sizeof ring);
    ep_model_setup(&phy_model, 0);
    struct ed_bus pb = ep_model_bus(&phy_model);
    CHECK(ep_init(&phy, &pb, 168000000, 0, 4, 100) == ED_OK);
    CHECK(ep_begin(&phy, 16, 1000) == ED_BUSY);
    int e = ED_BUSY;
    for (unsigned i = 0; i < 16 && e == ED_BUSY; i++)
    {
        e = ep_step(&phy);
    }
    CHECK(e == ED_OK);
    en_model_init(&model, &memory, 0x20001000);
    struct en_bus bus = en_model_bus(&model);
    CHECK(en_init(&ring, &memory, 0x20001000, &bus, &phy, local_mac) == ED_OK);
    CHECK(en_reset_begin(&ring, 0, 4, 100) == ED_BUSY);
    CHECK(en_reset_step(&ring, 1) == ED_BUSY &&
          en_reset_step(&ring, 2) == ED_BUSY &&
          en_reset_step(&ring, 3) == ED_OK && en_start(&ring) == ED_OK);
    CHECK(nb_init(&net, &ring, config, 1, 1000, 16, 0) == NB_OK);
    return 0;
}

static int inject(const unsigned char *ip, size_t length)
{
    unsigned char f[1518] = {0};
    memcpy(f, local_mac, 6);
    memcpy(f + 6, peer_mac, 6);
    nc_w16(f + 12, 0x800);
    memcpy(f + 14, ip, length);
    size_t wire = 14 + length;
    if (wire < 60)
    {
        wire = 60;
    }
    return en_model_rx(&model, f, wire + 4,
                       ((uint32_t)(wire + 4) << 16) | 0x300u);
}

static int arp_reply(void)
{
    unsigned char f[64] = {0};
    memcpy(f, local_mac, 6);
    memcpy(f + 6, peer_mac, 6);
    nc_w16(f + 12, 0x806);
    nc_w16(f + 14, 1);
    nc_w16(f + 16, 0x800);
    f[18] = 6;
    f[19] = 4;
    nc_w16(f + 20, 2);
    memcpy(f + 22, peer_mac, 6);
    memcpy(f + 28, config.peer_ip, 4);
    memcpy(f + 32, local_mac, 6);
    memcpy(f + 38, config.local_ip, 4);
    return en_model_rx(&model, f, 64, (64u << 16) | 0x300u);
}

static int handshake(unsigned ack_before_dma)
{
    unsigned char p[300];
    size_t n = nc_wire(p, 30, 0, 2, 512, 0, 0, 100);
    CHECK(inject(p, n) == ED_OK);
    CHECK(nb_step(&net, 0) == NB_OK);
    CHECK(net.pending_tcp && !net.tcp.flight && net.tcp.snd_nxt == 1000 &&
          net.arp.requests == 1);
    CHECK(en_model_tx(&model, 0) == ED_OK && nc_u16(model.wire + 12) == 0x806);
    CHECK(arp_reply() == ED_OK && nb_step(&net, 1) == NB_OK);
    CHECK(net.tcp.flight && net.tcp.dma && net.tcp.snd_nxt == 1001 &&
          net.tcp.due == 1001 && !net.pending_tcp);
    n = nc_wire(p, 31, 1001, 16, 512, 0, 0, 0);
    if (ack_before_dma)
    {
        CHECK(inject(p, n) == ED_OK && nb_step(&net, 2) == NB_OK);
        CHECK(!net.tcp.flight && net.tcp.dma &&
              net.tcp.state == NC_ESTABLISHED);
        CHECK(en_model_tx(&model, 0) == ED_OK);
        CHECK(nb_step(&net, 3) == NB_OK && !net.tcp.dma);
    }
    else
    {
        CHECK(en_model_tx(&model, 0) == ED_OK && nb_step(&net, 2) == NB_OK);
        CHECK(net.tcp.flight && !net.tcp.dma);
        CHECK(inject(p, n) == ED_OK && nb_step(&net, 3) == NB_OK &&
              net.tcp.state == NC_ESTABLISHED && !net.tcp.flight);
    }
    CHECK(net.rx_frames == net.rx_released && !model.violation &&
          !phy_model.violation);
    return 0;
}

static int ownership(void)
{
    CHECK(!boot() && !handshake(0));
    CHECK(!boot() && !handshake(1));
    unsigned char data[32];
    for (unsigned i = 0; i < 32; i++)
    {
        data[i] = (unsigned char)(i + 1);
    }
    CHECK(nb_offer(&net, data, 32) == NB_OK && nb_step(&net, 4) == NB_OK &&
          net.tcp.flight && net.tcp.dma);
    data[0] = 99;
    CHECK(memory.tx_bytes[0][54] == 1 || memory.tx_bytes[1][54] == 1);
    CHECK(en_model_tx(&model, 1) == ED_OK && nb_step(&net, 5) == NB_OK);
    CHECK(net.tx_errors == 1 && net.tcp.flight && !net.tcp.dma);
    CHECK(!boot());
    /* Actual valid SYN prepares one TCP packet while ARP owns its retained copy.
     * A new SYN during resolution must be dropped without invalidating it. */
    unsigned char p[300];
    size_t n = nc_wire(p, 30, 0, 2, 512, 0, 0, 100);
    CHECK(inject(p, n) == ED_OK && nb_step(&net, 0) == NB_OK);
    struct nc_ticket before = net.offered.ticket;
    n = nc_wire(p, 30, 0, 2, 512, 0, 0, 100);
    CHECK(inject(p, n) == ED_OK && nb_step(&net, 1) == NB_OK);
    CHECK(net.paused_tcp == 1 && net.pending_tcp && net.tcp.pending_valid &&
          net.offered.ticket.serial == before.serial);
    CHECK(nb_offer(&net, data, 4) == NB_BUSY);
    CHECK(nb_step(&net, 3000) == NB_WAIT && net.tcp.state == NC_CLOSED &&
          !net.pending_tcp && !net.tcp.flight);
    return 0;
}

static int pressure(void)
{
    CHECK(!boot() && !handshake(0));
    /* Two real descriptor copies from unrelated control frames occupy both slots.
     * Ring is single-producer in normal use; test installs its own valid records.
     */
    unsigned char f[60] = {0};
    for (unsigned i = 0; i < 2; i++)
    {
        struct en_ticket t;
        CHECK(en_tx_copy(&ring, f, sizeof f, &t) == ED_OK &&
              en_tx_publish(&ring, t) == ED_OK);
        net.copies[t.slot] = (struct nb_copy){1, 0, t, {0, 0}};
    }
    unsigned char payload[8] = {1, 2, 3};
    CHECK(nb_offer(&net, payload, 8) == NB_OK);
    CHECK(nb_step(&net, 4) == NB_WAIT);
    CHECK(net.pending_tcp && !net.tcp.flight && !net.tcp.dma &&
          net.tcp.snd_nxt == 1001);
    CHECK(en_model_tx(&model, 0) == ED_OK && nb_step(&net, 5) == NB_OK);
    CHECK(net.tcp.flight && net.tcp.dma && net.tcp.sent_at == 5 &&
          net.tcp.due == 1005);
    return 0;
}

static int stop_ownership(void)
{
    CHECK(!boot() && !handshake(0));
    unsigned char data[4] = {1, 2, 3, 4};
    CHECK(nb_offer(&net, data, 4) == NB_OK && nb_step(&net, 4) == NB_OK);
    struct nc_ticket copy =
        net.copies[0].tcp ? net.copies[0].transport : net.copies[1].transport;
    model.stop_stuck = 1;
    phy_model.phy[0][1] = 0x20;
    CHECK(nb_step(&net, 5) == NB_WAIT);
    CHECK(net.phase == NB_STOPPING && net.tcp.state == NC_CLOSED &&
          net.tcp.dma && !net.tcp.flight);
    CHECK(nb_stop_step(&net, 6) == NB_WAIT && net.tcp.dma);
    CHECK(nb_restart(&net, 2, 5000, 16, 6) == NB_BUSY);
    model.reg[0x1014 / 4] &= ~0x7e0000u;
    CHECK(nb_stop_step(&net, 7) == NB_STOPPED && !net.tcp.dma);
    CHECK(nc_dma_done(&net.tcp, copy) == NC_STALE);
    CHECK(nb_restart(&net, 2, 5000, 16, 8) == NB_BUSY);
    /* Requalify actual PHY and reset actual descriptor epoch before rebind. */
    phy_model.phy[0][1] = 0x24;
    CHECK(ep_begin(&phy, 16, 1000) == ED_BUSY);
    int e = ED_BUSY;
    for (unsigned i = 0; i < 16 && e == ED_BUSY; i++)
    {
        e = ep_step(&phy);
    }
    CHECK(e == ED_OK);
    CHECK(en_reset_begin(&ring, 8, 4, 100) == ED_BUSY &&
          en_reset_step(&ring, 9) == ED_BUSY &&
          en_reset_step(&ring, 10) == ED_BUSY &&
          en_reset_step(&ring, 11) == ED_OK && en_start(&ring) == ED_OK);
    CHECK(nb_restart(&net, 1, 5000, 16, 12) == NB_BUSY);
    CHECK(nb_restart(&net, 2, 5000, 16, 12) == NB_OK && net.tcp.epoch == 2 &&
          net.tcp.state == NC_LISTEN && !net.arp.known);
    CHECK(!boot() && !handshake(0));
    CHECK(nb_offer(&net, data, 4) == NB_OK && nb_step(&net, 4) == NB_OK);
    model.stop_stuck = 1;
    CHECK(nb_stop(&net, 5) == NB_WAIT);
    CHECK(nb_stop_step(&net, 106) == NB_FAULT && net.tcp.dma &&
          net.phase == NB_FAILED);
    CHECK(nb_restart(&net, 2, 5000, 16, 107) == NB_BUSY);
    return 0;
}

static int boundaries(void)
{
    CHECK(!boot() && !handshake(0));
    unsigned char p[300];
    size_t n = nc_wire(p, 31, 1001, 16, 512, 0, 0, 0);
    /* A borrowed frame stalls its slot across DMA wrap. It may be returned during
     * stop, but that return must not republish OWN to a stopping engine. */
    CHECK(inject(p, n) == ED_OK);
    struct en_frame held;
    CHECK(en_rx_take(&ring, &held) == ED_OK);
    unsigned char saved[60];
    memcpy(saved, held.bytes, 60);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(inject(p, n) == ED_OK);
        struct en_frame f;
        CHECK(en_rx_take(&ring, &f) == ED_OK &&
              en_rx_release(&ring, f.ticket) == ED_OK);
    }
    CHECK(inject(p, n) == ED_BUSY && !memcmp(held.bytes, saved, 60));
    CHECK(nb_stop(&net, UINT64_C(4294967295)) == NB_WAIT);
    CHECK(nb_stop_step(&net, UINT64_C(4294967296)) == NB_WAIT);
    CHECK(en_rx_release(&ring, held.ticket) == ED_OK &&
          !(memory.rx[held.ticket.slot].word[0] & 0x80000000u));
    CHECK(nb_stop_step(&net, UINT64_C(4294967297)) == NB_STOPPED);
    CHECK(en_rx_release(&ring, held.ticket) == ED_STALE);
    /* A later deadline beyond the32-bit horizon fails closed without wrap. */
    CHECK(!boot() && !handshake(0));
    model.stop_stuck = 1;
    CHECK(nb_stop(&net, UINT64_C(4294967295)) == NB_WAIT);
    CHECK(nb_stop_step(&net, UINT64_C(8589934592)) == NB_FAULT);
    puts("NB_BOUNDARIES_PASS held_RX_wrap_stop_64bit_deadline");
    return 0;
}

static void echo_packet(unsigned char p[84])
{
    memset(p, 0, 84);
    p[0] = 0x45;
    nc_w16(p + 2, 84);
    nc_w16(p + 4, 123);
    p[8] = 64;
    p[9] = 1;
    nc_w32(p + 12, 0xc0000201);
    nc_w32(p + 16, 0xc0000202);
    p[20] = 8;
    nc_w16(p + 24, 0x1234);
    nc_w16(p + 26, 7);
    for (unsigned i = 0; i < 56; i++)
    {
        p[28 + i] = (unsigned char)(i ^ 0xa5);
    }
    nc_w16(p + 22, nc_sum(p + 20, 64, 0));
    nc_w16(p + 10, nc_sum(p, 20, 0));
}

static size_t fragment(unsigned char *out, const unsigned char *packet,
                       unsigned offset, unsigned count, unsigned more)
{
    memcpy(out, packet, 20);
    memcpy(out + 20, packet + 20 + offset, count);
    nc_w16(out + 2, count + 20);
    nc_w16(out + 6, (more ? 0x2000 : 0) | (offset / 8));
    nc_w16(out + 10, 0);
    nc_w16(out + 10, nc_sum(out, 20, 0));
    return count + 20;
}

static int ip_composition(void)
{
    unsigned char p[84], f[84];
    echo_packet(p);
    CHECK(!boot());
    CHECK(arp_reply() == ED_OK && nb_step(&net, 0) == NB_OK);
    CHECK(inject(p, 84) == ED_OK && nb_step(&net, 1) == NB_OK);
    CHECK(en_model_tx(&model, 0) == ED_OK && model.wire_length == 98 &&
          nc_u16(model.wire + 12) == 0x800 && model.wire[34] == 0 &&
          !nc_sum(model.wire + 34, 64, 0) &&
          !memcmp(model.wire + 42, p + 28, 56));
    CHECK(nb_step(&net, 2) == NB_OK && net.rx_frames == net.rx_released);
    /* Final-first fragments use actual two-slot reassembly before Echo. */
    nc_w16(p + 4, 124);
    nc_w16(p + 10, 0);
    nc_w16(p + 10, nc_sum(p, 20, 0));
    size_t n = fragment(f, p, 24, 40, 0);
    CHECK(inject(f, n) == ED_OK && nb_step(&net, 3) == NB_OK);
    CHECK(!net.arp.pending);
    n = fragment(f, p, 0, 24, 1);
    CHECK(inject(f, n) == ED_OK && nb_step(&net, 4) == NB_OK);
    CHECK(en_model_tx(&model, 0) == ED_OK && model.wire[34] == 0 &&
          model.wire_length == 98 && !memcmp(model.wire + 42, p + 28, 56));
    CHECK(nb_step(&net, 5) == NB_OK);
    /* At the exact absolute deadline, expire/report BEFORE a final fragment can
     * internally tick away the saved first-header evidence. */
    nc_w16(p + 4, 125);
    nc_w16(p + 10, 0);
    nc_w16(p + 10, nc_sum(p, 20, 0));
    n = fragment(f, p, 0, 24, 1);
    CHECK(inject(f, n) == ED_OK && nb_step(&net, 10) == NB_OK);
    /* Refresh the actual neighbor mapping shortly before its30s lifetime ends. */
    CHECK(arp_reply() == ED_OK && nb_step(&net, 59000) == NB_OK);
    n = fragment(f, p, 24, 40, 0);
    CHECK(inject(f, n) == ED_OK && nb_step(&net, 60010) == NB_OK);
    CHECK(en_model_tx(&model, 0) == ED_OK && model.wire_length == 70 &&
          model.wire[34] == 11 && model.wire[35] == 1 &&
          nc_u16(model.wire + 46) == 125);
    CHECK(nb_step(&net, 60011) == NB_OK && net.rx_frames == net.rx_released &&
          !model.violation);
    /* Malformed upper input still releases every real borrowed descriptor. */
    for (unsigned i = 0; i < 12; i++)
    {
        p[10] ^= 1;
        CHECK(inject(p, 84) == ED_OK && nb_step(&net, 60012 + i) == NB_OK);
        p[10] ^= 1;
    }
    CHECK(net.rx_rejected == 12 && net.rx_frames == net.rx_released);
    return 0;
}

static int write_failure(void)
{
    CHECK(!boot() && !handshake(0));
    unsigned char data[8] = {0};
    CHECK(nb_offer(&net, data, 8) == NB_OK);
    /* Status read+settle+write consume3 bus calls; publication then fails its
     * poll-demand write after OWN has already been set. Stop request succeeds. */
    model.fail_at = model.calls + 5;
    CHECK(nb_step(&net, 4) == NB_WAIT && net.phase == NB_STOPPING &&
          net.tcp.dma && net.tcp.state == NC_CLOSED);
    CHECK(memory.tx[0].word[0] & 0x80000000u ||
          memory.tx[1].word[0] & 0x80000000u);
    CHECK(nb_stop_step(&net, 5) == NB_STOPPED && !net.tcp.dma);
    CHECK(!boot());
    model.reg[0x1014 / 4] |= 0x2000;
    CHECK(nb_step(&net, 0) == NB_WAIT && net.tcp.state == NC_CLOSED);
    CHECK(nb_stop_step(&net, 1) == NB_STOPPED);
    return 0;
}

int main(void)
{
    /* The retained helper independently verifies the predecessor handshake. */
    struct nc_stack prior;
    CHECK(!nc_establish(&prior, 16, 1000, 30, 100));
    CHECK(!ownership());
    CHECK(!pressure());
    CHECK(!stop_ownership());
    CHECK(!ip_composition());
    CHECK(!write_failure());
    CHECK(!boundaries());
    puts("NB_PASS actual_EnRing_ARP_TCP_copy_pressure_ACK_DMA_link_quiescence");
    return 0;
}
