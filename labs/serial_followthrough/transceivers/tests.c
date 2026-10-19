#include "transceivers.h"
#include "uart_depth.h"
#include <limits.h>
void tr_note(const char *message);
void tr_values(const char *case_name, unsigned a, unsigned b, unsigned c, unsigned d);
#define CHECK(x) do { if (!(x)) { tr_note("TR_ASSERT: " #x); return 41; } } while (0)

static int thresholds(void) {
    TrSample s = {TR_RS232_FIXTURE, 1, 0, 0, -6000, 0}; int d = 77;
    CHECK(tr_sample(&s, &d) == TR_OK && d == 1);
    s.a_mv = 0; CHECK(tr_sample(&s, &d) == TR_OK && d == TR_UNKNOWN);
    s.a_mv = -3000; CHECK(tr_sample(&s, &d) == TR_OK && d == 1);
    s.a_mv = 3000; CHECK(tr_sample(&s, &d) == TR_OK && d == 0);
    unsigned points = 0, changed = 0;
    for (int32_t v = -25000; v <= 25000; ++v) {
        s.device = TR_MAX3232_33; s.a_mv = v;
        CHECK(tr_sample(&s, &d) == TR_OK);
        int expected = v <= 600 ? 1 : v >= 2400 ? 0 : TR_UNKNOWN;
        CHECK(d == expected); ++points;
        s.device = TR_RS232_FIXTURE; CHECK(tr_sample(&s, &d) == TR_OK);
        if (d != expected) ++changed;
    }
    CHECK(points == 50001 && changed == 4200);
    s.device = TR_MAX3232_33; s.open = 1; s.a_mv = 0;
    CHECK(tr_sample(&s, &d) == TR_OK && d == 1);
    d = 77; s.a_mv = INT32_MIN; CHECK(tr_sample(&s, &d) == TR_INVALID && d == 77);
    s.a_mv = 0; s.enabled = 2; CHECK(tr_sample(&s, &d) == TR_INVALID && d == 77);
    tr_values("thresholds", points, changed, 600, 2400); return 0;
}
static int connections(void) {
    TrConnector dte = {TR_DTE,1,2,3}, dce = {TR_DCE,1,2,3};
    const TrWire straight[] = {{1,1},{2,2},{3,3}}, crossed[] = {{1,2},{2,1},{3,3}};
    CHECK(tr_connections(&dte,&dce,straight,3) == TR_OK);
    CHECK(tr_connections(&dte,&dte,straight,3) == TR_INVALID);
    CHECK(tr_connections(&dte,&dte,crossed,3) == TR_OK);
    CHECK(tr_connections(&dte,&dce,crossed,3) == TR_INVALID);
    CHECK(tr_connections(&dte,&dce,straight,2) == TR_INVALID);
    unsigned accepted = 0;
    for (unsigned a=1;a<=3;++a) for (unsigned b=1;b<=3;++b) for (unsigned c=1;c<=3;++c) {
        TrWire w[]={{1,a},{2,b},{3,c}};
        int r=tr_connections(&dte,&dce,w,3);
        CHECK((r == TR_OK) == (a == 1 && b == 2 && c == 3));
        accepted += r == TR_OK;
    }
    Tr232Circuit circuit={3300,115200,3000,1000,{100,100,100,100},100,25};
    CHECK(tr_232_circuit(&circuit) == TR_OK);
    circuit.baud=250000; CHECK(tr_232_circuit(&circuit) == TR_INVALID);
    circuit.baud=9600; circuit.capacitance_pf=2500;
    CHECK(tr_232_circuit(&circuit) == TR_INVALID);
    circuit.capacitance_pf=1000; circuit.pump_nf[0]=47;
    CHECK(tr_232_circuit(&circuit) == TR_INVALID);
    tr_values("connections",27,accepted,3,115200); return 0;
}
typedef struct { uint32_t regs[16]; unsigned count; uint8_t bytes[16]; } TrUart;
static uint32_t read_uart(void *context, unsigned k) {
    TrUart *m=context; return k == PU_SOURCE ? 1 : m->regs[k];
}
static void write_uart(void *context,unsigned k,uintptr_t v) {
    TrUart *m=context; m->regs[k]=(uint32_t)v;
    if (k == PU_DR) { if(m->count<16)m->bytes[m->count++]=(uint8_t)v; m->regs[PU_SR]&=~(PU_TXE|PU_TC); }
}
static int capture232(void) {
    TrPoint p[160]; TrResult result; unsigned bits=0;
    for (unsigned value=0; value<256; ++value) {
        UaFormat f; CHECK(ua_format(42000000,115200,0,8,0,1,(uint16_t)value,&f) == PU_OK);
        for(unsigned i=0;i<10;++i) {
            p[i]=(TrPoint){i*10000u,f.wire.bits[i]?-6000:6000,0,f.wire.bits[i],f.wire.bits[i],7};
        }
        CHECK(tr_capture(TR_MAX3232_33,p,10,0,&result) == TR_OK && result.matched == 10);
        bits+=10;
    }
    p[5].columns=TR_HAVE_LOGIC|TR_HAVE_RETURN; result.matched=99;
    CHECK(tr_capture(TR_MAX3232_33,p,10,0,&result) == TR_INCOMPLETE && result.matched == 99);
    p[5].columns=7; p[5].a_mv=1500;
    CHECK(tr_capture(TR_MAX3232_33,p,10,0,&result) == TR_MISMATCH && result.unknown == 1 && result.first_bad == 5);
    p[5].a_mv=6000;
    CHECK(tr_capture(TR_MAX3232_33,p,10,0,&result) == TR_MISMATCH && result.mismatched == 1);
    UaTx tx={0}; TrUart m={0}; PuIo io={&m,read_uart,write_uart}; PuClock clock={42000000,1,1,1,1,0};
    uint8_t bytes[16]; for(unsigned i=0;i<16;++i)bytes[i]=(uint8_t)(0xa5u^i);
    CHECK(pu_open(&tx.byte,io,&clock,115200,0,1) == PU_OK);
    CHECK(ua_begin(&tx,io,1,1,bytes,16,0,100) == PU_OK);
    unsigned now=0;
    while(tx.active && now<50) {
        m.regs[PU_SR]=PU_TXE|PU_TC;
        int r=ua_step(&tx,io,1,now++); CHECK(r == PU_BUSY || r == PU_OK);
    }
    CHECK(!tx.active && m.count == 16 && tx.offset == 16 && tx.byte.token == 2);
    for(unsigned i=0;i<16;++i)CHECK(m.bytes[i] == bytes[i]);
    tr_values("capture232",bits,m.count,tx.byte.token,5); return 0;
}
static int differential(void) {
    TrSample s={TR_RS422_FIXTURE,1,0,0,200,0}; int d;
    CHECK(tr_sample(&s,&d) == TR_OK && d == 1);
    s.a_mv=199; CHECK(tr_sample(&s,&d) == TR_OK && d == TR_UNKNOWN);
    unsigned points=0;
    for(int32_t v=-7000;v<=7000;++v) {
        s.device=TR_AM26LV32E; s.a_mv=v;
        for(unsigned sign=0;sign<2;++sign) {
            s.swapped=sign; CHECK(tr_sample(&s,&d) == TR_OK);
            int64_t delta=sign?-(int64_t)v:v;
            CHECK(d == (delta>=200?1:delta<=-200?0:TR_UNKNOWN)); ++points;
        }
    }
    s.a_mv=0;s.swapped=0;s.open=1;
    CHECK(tr_sample(&s,&d) == TR_OK && d == 1);
    s.open=0;CHECK(tr_sample(&s,&d) == TR_OK && d == TR_UNKNOWN);
    s.enabled=0;CHECK(tr_sample(&s,&d) == TR_OK && d == TR_HIGH_Z);
    s.enabled=1;s.a_mv=7100;s.b_mv=6900;CHECK(tr_sample(&s,&d) == TR_OK && d == 1);
    s.a_mv=7101;d=77;CHECK(tr_sample(&s,&d) == TR_INVALID && d == 77);
    CHECK(tr_topology(1,2,1,1) == TR_OK && tr_topology(2,2,1,1) == TR_INVALID);
    CHECK(tr_topology(1,11,1,1) == TR_INVALID && tr_topology(1,2,0,1) == TR_INVALID);
    tr_values("differential",points,10,200,7000); return 0;
}
static int exclusion(void) {
    TrBus b={0};
    CHECK(tr_grant(&b,0,1,0) == TR_OK);
    CHECK(tr_grant(&b,1,2,1) == TR_BUSY && b.owner == 0 && b.serial == 1);
    CHECK(tr_release(&b,0,1,100601,0,200,2000) == TR_BUSY && b.phase == 1);
    CHECK(tr_release(&b,0,1,200600,1,200,2000) == TR_OK && b.ready_at == 202800);
    CHECK(tr_grant(&b,1,2,202799) == TR_BUSY);
    CHECK(tr_ready(&b,202799) == TR_BUSY && tr_ready(&b,202800) == TR_OK);
    CHECK(tr_grant(&b,1,2,202800) == TR_OK);
    CHECK(tr_release(&b,0,1,202801,1,0,0) == TR_STALE && b.owner == 1);
    CHECK(tr_release(&b,1,2,UINT64_MAX,1,1,0) == TR_INVALID && b.phase == 1);
    CHECK(tr_release(&b,1,2,202801,1,0,0) == TR_OK);
    CHECK(tr_ready(&b,202801) == TR_OK);
    CHECK(tr_grant(&b,2,2,202802) == TR_STALE);
    CHECK(tr_grant(&b,2,UINT32_MAX,202802) == TR_OK);
    CHECK(tr_release(&b,2,UINT32_MAX,202803,1,0,0) == TR_OK);
    CHECK(tr_ready(&b,202803) == TR_OK);
    CHECK(tr_grant(&b,0,1,202804) == TR_STALE);
    tr_values("exclusion",202800,2,UINT32_MAX,0);return 0;
}
static int captures422(void) {
    TrPoint clean[5], swapped[5], ringing[5];
    const int32_t a[]={0,2000,0,2000,2000};
    const int32_t b[]={2000,0,2000,0,0};
    for(unsigned i=0;i<5;++i) {
        clean[i]=(TrPoint){i*100u,2000,0,1,1,15};
        swapped[i]=(TrPoint){i*100u,0,2000,1,0,15};
        ringing[i]=(TrPoint){i*100u,a[i],b[i],1,a[i]>b[i],15};
    }
    TrResult r;TrEdge edge;
    CHECK(tr_capture(TR_AM26LV32E,clean,5,0,&r) == TR_OK && r.matched == 5);
    CHECK(tr_capture(TR_AM26LV32E,swapped,5,0,&r) == TR_MISMATCH && r.mismatched == 5);
    CHECK(tr_edge(swapped,5,1,0,&edge) == TR_OK && edge.opposite == 5 && edge.transitions == 0);
    CHECK(tr_edge(swapped,5,1,1,&edge) == TR_OK && edge.opposite == 0);
    CHECK(tr_edge(ringing,5,1,0,&edge) == TR_OK && edge.transitions == 3 && edge.opposite == 2);
    ringing[2].a_mv+=10000;ringing[2].b_mv+=10000;
    CHECK(tr_edge(ringing,5,1,0,&edge) == TR_OK && edge.outside == 1);
    ringing[2].time_ns=ringing[1].time_ns;edge.outside=99;
    CHECK(tr_edge(ringing,5,1,0,&edge) == TR_INVALID && edge.outside == 99);
    tr_values("captures422",5,0,3,1);return 0;
}
int tr_test(unsigned selector) {
    switch(selector) {
    case 0:return thresholds(); case 1:return connections(); case 2:return capture232();
    case 3:return differential(); case 4:return exclusion(); case 5:return captures422();
    default:return 42;
    }
}
