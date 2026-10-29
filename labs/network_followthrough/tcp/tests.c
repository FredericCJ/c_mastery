#include "test_wire.h"

int main(void)
{
    struct nc_rtt r;
    CHECK(nc_rtt_init(&r, 1, 0) == NC_OK);
    CHECK(nc_rtt_sample(&r, 80, 0) == NC_OK);
    CHECK(r.srtt == 80 && r.variance == 40 && r.rto == 1000);
    CHECK(nc_rtt_sample(&r, 120, 0) == NC_OK);
    CHECK(r.srtt == 85 && r.variance == 40 && r.rto == 1000);
    CHECK(nc_rtt_sample(&r, 800, 1) == NC_WAIT);
    CHECK(r.samples == 2 && r.srtt == 85);
    struct nc_stack s;
    struct nc_packet out;
    unsigned char p[1500], data[256];
    for (unsigned i = 0; i < 256; i++)
    {
        data[i] = (unsigned char)i;
    }
    CHECK(!nc_establish(&s, 512, 0xfffffff0, 0xfffffff0, 100));
    CHECK(s.mss == 100);
    size_t n = nc_wire(p, 0xfffffff1, s.snd_nxt, 0x18, 512, data, 16, 0);
    CHECK(nc_receive(&s, p, n, 81) == NC_OK);
    CHECK(s.rcv_nxt == 1 && s.rx.count == 16);
    CHECK(nc_receive(&s, p, n, 82) == NC_WAIT);
    CHECK(s.rcv_nxt == 1 && s.rx.count == 16);
    n = nc_wire(p, 0xfffffff9, s.snd_nxt, 0x18, 512, data, 12, 0);
    CHECK(nc_receive(&s, p, n, 83) == NC_OK);
    CHECK(s.rcv_nxt == 5 && s.rx.count == 20);
    CHECK(nc_prepare(&s, &out) == NC_OK);
    CHECK(nc_u16(out.bytes + 34) == 492);
    CHECK(nc_submit(&s, out.ticket, 0, 2000) == NC_WAIT);
    CHECK(!s.flight && s.snd_nxt == 0xfffffff1);
    CHECK(nc_submit(&s, out.ticket, 1, 2001) == NC_OK);
    CHECK(nc_dma_done(&s, out.ticket) == NC_OK);
    CHECK(nc_offer(&s, data, 200) == NC_OK);
    CHECK(nc_prepare(&s, &out) == NC_OK);
    CHECK(out.length == 140);
    CHECK(nc_submit(&s, out.ticket, 0, 5000) == NC_WAIT);
    CHECK(!s.flight);
    CHECK(nc_submit(&s, out.ticket, 1, 5001) == NC_OK);
    CHECK(s.due == 6001 && s.snd_nxt == 85);
    CHECK(nc_dma_done(&s, out.ticket) == NC_OK);
    CHECK(s.flight && s.due == 6001);
    CHECK(nc_tick(&s, 6000) == NC_OK);
    CHECK(nc_prepare(&s, &out) == NC_WAIT);
    CHECK(nc_tick(&s, 6001) == NC_OK);
    CHECK(nc_prepare(&s, &out) == NC_OK);
    CHECK(s.retries == 0);
    CHECK(nc_submit(&s, out.ticket, 1, 6001) == NC_OK);
    CHECK(s.retries == 1 && s.due == 8001);
    CHECK(nc_dma_done(&s, out.ticket) == NC_OK);
    n = nc_wire(p, s.rcv_nxt, s.snd_nxt, 16, 0, 0, 0, 0);
    CHECK(nc_receive(&s, p, n, 6100) == NC_OK);
    CHECK(!s.flight && s.rtt.samples == 1);
    CHECK(nc_tick(&s, 8100) == NC_OK);
    CHECK(nc_prepare(&s, &out) == NC_OK);
    CHECK(s.pending_kind == NC_PROBE);
    uint32_t seq = s.snd_nxt;
    size_t at = s.tx_at;
    CHECK(nc_submit(&s, out.ticket, 1, 8100) == NC_OK);
    CHECK(s.snd_nxt == seq && s.tx_at == at && !s.flight);
    CHECK(nc_dma_done(&s, out.ticket) == NC_OK);
    CHECK(nc_tick(&s, UINT64_MAX) == NC_RANGE);
    CHECK(!nc_establish(&s, 4, 20, 30, 256));
    n = nc_wire(p, 31, 21, 0x18, 512, data, 8, 0);
    CHECK(nc_receive(&s, p, n, 81) == NC_OK);
    CHECK(s.rcv_nxt == 35 && s.rx.count == 4);
    CHECK(nc_prepare(&s, &out) == NC_OK);
    CHECK(nc_u16(out.bytes + 34) == 0);
    CHECK(nc_consume(&s, 2) == NC_OK);
    CHECK(nc_submit(&s, out.ticket, 1, 82) == NC_STALE);
    CHECK(nw_rx_window(&s.rx) == 2);
    CHECK(nc_prepare(&s, &out) == NC_OK);
    CHECK(nc_u16(out.bytes + 34) == 2);
    CHECK(!nc_establish(&s, 16, 20, 30, 256));
    n = nc_wire(p, 32, 0, 4, 512, 0, 0, 0);
    CHECK(nc_receive(&s, p, n, 81) == NC_WAIT);
    CHECK(s.state == NC_ESTABLISHED && s.ack_needed);
    n = nc_wire(p, 47, 0, 4, 512, 0, 0, 0);
    s.ack_needed = 0;
    CHECK(nc_receive(&s, p, n, 82) == NC_WAIT);
    CHECK(!s.ack_needed && s.state == NC_ESTABLISHED);
    n = nc_wire(p, 31, 0, 4, 512, 0, 0, 0);
    CHECK(nc_receive(&s, p, n, 83) == NC_ABORT);
    CHECK(s.state == NC_CLOSED);
    puts("NC_PASS wrap_overlap_capacity_acceptance_RTO_Karn_reset");
    return 0;
}
