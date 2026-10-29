#include "session.h"
#include <limits.h>
#include <string.h>
int puts(const char *);

static void hex(const uint8_t *p, size_t n)
{
    char text[161];
    const char digits[] = "0123456789abcdef";
    for (size_t i = 0; i < n; i++)
    {
        text[2 * i] = digits[p[i] >> 4];
        text[2 * i + 1] = digits[p[i] & 15];
    }
    text[2 * n] = 0;
    puts(text);
}

#define CHECK(x)                   \
    do                             \
    {                              \
        if (!(x))                  \
        {                          \
            puts("ES_ASSERT:" #x); \
            return 41;             \
        }                          \
    } while (0)
static uint8_t negotiation[3] = {1, 1, 17};

static int move(struct es_peer *from, struct es_peer *to, uint32_t *time,
                unsigned drop)
{
    for (unsigned k = 0; k < 20; k++)
    {
        uint8_t wire[40], ack[40];
        size_t n, z;
        uint32_t token;
        int r = es_offer(from, *time, wire, sizeof wire, &n, &token);
        if (r == E7_WAIT)
        {
            *time += 100;
            continue;
        }
        CHECK(r == E7_OK);
        CHECK(es_admit(from, from->tx.identity.epoch, token, 1, *time) ==
              E7_OK);
        r = es_receive(to, wire, n, *time, ack, sizeof ack, &z);
        CHECK(r == E7_OK || r == ET_COMPLETE);
        if (drop && r == ET_COMPLETE)
        {
            return 0;
        }
        CHECK(es_ack(from, ack, z, *time) == E7_OK);
        (*time)++;
        if (!from->tx.inner.active)
        {
            return 0;
        }
    }
    CHECK(0);
    return 41;
}

static int pair(struct es_peer *a, struct es_peer *b, uint32_t *now)
{
    struct es_event e;
    CHECK(es_init(a, 17, 2, 1) == 0 && es_init(b, 51, 2, 1) == 0);
    CHECK(es_emit(a, 1, 7, 1, 0, negotiation, 3, 1, *now) == 0 &&
          !es_permitted(a));
    CHECK(move(a, b, now, 0) == 0 && es_deliver(b, 80, &e) == ES_DELIVERED &&
          b->phase == ES_ACK_NEEDED);
    CHECK(es_emit(b, 2, 7, 1, 0, negotiation, 3, 1, *now) == 0 &&
          move(b, a, now, 0) == 0);
    CHECK(es_deliver(a, 80, &e) == ES_DELIVERED && es_permitted(a) &&
          !es_permitted(b));
    return 0;
}

static int encode(unsigned verb, unsigned req, int value, uint8_t *p, size_t *n)
{
    struct edu_record r = {.verb = (uint8_t)verb,
                           .request = (uint16_t)req,
                           .value = (int16_t)value,
                           .has_value = verb != 1,
                           .has_unit = verb != 1};
    return edu_record_encode(&r, p, n);
}

static int token(struct es_peer *a, struct es_peer *b, uint16_t id,
                 uint32_t *now, unsigned drop)
{
    uint8_t next = (uint8_t)b->self;
    CHECK(es_emit(a, 5, 7, 1, 0, &next, 1, id, *now) == 0);
    CHECK(move(a, b, now, drop) == 0);
    struct es_event e;
    CHECK(es_deliver(b, 80, &e) == ES_DELIVERED);
    return 0;
}

/* Deliberately bypass local emission validation while still traversing actual
   transport/reassembly. This is original synthetic input, not a foreign target. */
static int raw(struct es_peer *from, struct es_peer *to, uint16_t id,
               const uint8_t *wire, size_t n, uint32_t *now)
{
    CHECK(et_begin(&from->tx, id, wire, n, *now) == 0);
    CHECK(move(from, to, now, 0) == 0);
    return 0;
}

static int dialogue(void)
{
    struct es_peer a = {0}, b = {0};
    uint32_t now = 0;
    CHECK(pair(&a, &b, &now) == 0);
    struct es_event e;
    CHECK(token(&a, &b, 2, &now, 1) == 0);
    CHECK(!es_permitted(&a) && es_permitted(&b) && a.core.token_pending == 51 &&
          b.core.holder == 51);
    uint8_t p[33];
    size_t n;
    CHECK(encode(3, 1, 1800, p, &n) == 0);
    CHECK(es_emit(&a, 3, 7, 1, 1, p, n, 3, now) == E7_BUSY);
    CHECK(es_emit(&b, 3, 7, 1, 1, p, n, 2, now) == 0 &&
          move(&b, &a, &now, 0) == 0);
    CHECK(es_deliver(&a, 80, &e) == E7_BUSY && a.responses == 0 &&
          a.rx.inner.ready);
    CHECK(move(&a, &b, &now, 0) == 0 && a.core.holder == 51 &&
          !a.core.token_pending);
    CHECK(es_deliver(&a, 80, &e) == ES_DELIVERED && a.responses == 1);
    CHECK(token(&b, &a, 3, &now, 0) == 0 && es_permitted(&a) &&
          !es_permitted(&b));
    struct es_peer c = {0}, d = {0};
    now = 0;
    CHECK(pair(&c, &d, &now) == 0 && token(&c, &d, 2, &now, 1) == 0);
    CHECK(es_uncertain(&c, 2, 2) == E7_STALE &&
          es_uncertain(&c, 1, 3) == E7_STALE);
    CHECK(es_uncertain(&c, 1, 2) == E7_BUSY && !es_permitted(&c) &&
          es_permitted(&d) && c.core.token_pending == 51);
    CHECK(move(&c, &d, &now, 0) == 0 && !es_permitted(&c) &&
          d.core.holder == 51);
    /* Incompatible OPEN is refused before negotiation; lower ACK is not authority. */
    struct es_peer x = {0}, y = {0};
    now = 0;
    CHECK(es_init(&x, 17, 1, 1) == 0 && es_init(&y, 51, 1, 1) == 0);
    uint8_t incompatible[] = {1, 0, 7, 0, 1, 0, 0, 1, 2, 17};
    CHECK(raw(&x, &y, 1, incompatible, sizeof incompatible, &now) == 0);
    CHECK(es_deliver(&y, 80, &e) == E7_REJECTED && !y.core.open &&
          y.phase == ES_CLOSED);
    CHECK(es_emit(&y, 7, 7, 2, 0, 0, 0, 1, now) == E7_STALE);
    CHECK(es_emit(&y, 7, 7, 1, 0, 0, 0, 1, now) == E7_OK);
    CHECK(y.tx.inner.length == 7 && y.tx.inner.bytes[0] == 7 &&
          y.tx.inner.bytes[5] == 0 && y.tx.inner.bytes[6] == 0);
    /* A matching ERROR carries no new token permission. */
    CHECK(es_emit(&x, 1, 7, 1, 0, negotiation, 3, 2, now) == E7_OK);
    CHECK(move(&y, &x, &now, 0) == 0 &&
          es_deliver(&x, 80, &e) == ES_DELIVERED && x.uncertain &&
          !es_permitted(&x));
    puts("ES_DIALOGUE_VALUES 0 1 1 0");
    puts("ES_DIALOGUE_PASS two_actual_peers_lost_ACK_no_timeout_authority");
    return 0;
}

static int receipts(void)
{
    struct es_peer a = {0}, b = {0};
    uint32_t now = 0;
    struct es_event e;
    uint8_t request[33], reply[80];
    size_t n, rn;
    CHECK(pair(&a, &b, &now) == 0 && encode(2, 9, 2500, request, &n) == 0);
    CHECK(es_emit(&a, 3, 7, 1, 1, request, n, 2, now) == 0 &&
          move(&a, &b, &now, 0) == 0);
    struct es_peer before;
    memcpy(&before, &b, sizeof b);
    memset(&e, 0x55, sizeof e);
    struct es_event unchanged;
    memcpy(&unchanged, &e, sizeof e);
    CHECK(es_deliver(&b, 1, &e) == E7_FULL && !memcmp(&before, &b, sizeof b) &&
          !memcmp(&unchanged, &e, sizeof e));
    CHECK(es_deliver(&b, 80, &e) == ES_DELIVERED && b.effects == 1 &&
          b.setpoint == 2500);
    CHECK(b.core.receipt[0].checkpoint == 1 &&
          b.core.receipt[1].checkpoint == 0);
    rn = e.length;
    memcpy(reply, e.reply, rn);
    CHECK(token(&a, &b, 3, &now, 0) == 0);
    CHECK(es_emit(&b, 3, 7, 1, 1, reply, rn, 2, now) == 0 &&
          move(&b, &a, &now, 0) == 0 && es_deliver(&a, 80, &e) == ES_DELIVERED);
    CHECK(a.core.receipt[1].checkpoint == 1 &&
          a.core.receipt[0].checkpoint == 0 && a.responses == 1 &&
          a.response.value == 2500);
    CHECK(token(&b, &a, 3, &now, 0) == 0);
    uint8_t duplicate[87] = {3, 0, 7, 0, 1, 0, 1};
    memcpy(duplicate + 7, request, n);
    CHECK(es_emit(&a, 3, 7, 1, 1, request, n, 4, now) == 0 &&
          move(&a, &b, &now, 0) == 0 &&
          es_deliver(&b, 80, &e) == ES_DUPLICATE && b.effects == 1 &&
          e.length == rn && !memcmp(e.reply, reply, rn));
    duplicate[7 + n - 1] ^= 1;
    CHECK(raw(&a, &b, 5, duplicate, n + 7, &now) == 0 &&
          es_deliver(&b, 80, &e) == E7_REJECTED && b.effects == 1);
    uint8_t malformed[] = {3, 0, 7, 0, 1, 0, 2, 1, 1, 1, 2};
    CHECK(raw(&a, &b, 6, malformed, sizeof malformed, &now) == 0);
    CHECK(es_deliver(&b, 80, &e) == E7_REJECTED &&
          b.core.receipt[0].checkpoint == 1 && b.effects == 1 &&
          b.setpoint == 2500);
    CHECK(b.core.pending_token == 0);
    /* Original per-direction headroom: no implicit wrap to checkpoint0. */
    a.sent_checkpoint = UINT16_MAX;
    CHECK(es_emit(&a, 3, 7, 1, 0, request, n, 7, now) == E7_STALE);
    puts("ES_REPLY_HEX");
    hex(reply, rn);
    puts("ES_RECEIPTS_VALUES 1 1 0 2500");
    puts("ES_RECEIPTS_PASS upper_commit_capacity_and_directional_receipts");
    return 0;
}

/* Intentionally vulnerable original teaching adapter: a synthetic actuator
   lives outside the resettable receipt state. No physical operation occurs. */
static int cut_effect(struct e7_session *state, unsigned *actuator,
                      unsigned cut)
{
    uint8_t request[33], response[33], cached[80];
    size_t n, z = 0;
    uint32_t token = 0;
    CHECK(encode(2, 9, 2500, request, &n) == 0);
    CHECK(e7_session_open(state, 7, 1, 1, 1) == E7_OK);
    CHECK(e7_session_prepare(state, 17, 7, 1, 1, request, n, &token, cached,
                             sizeof cached, &z) == E7_OK);
    struct edu_record record, reply;
    int16_t setpoint = 2200;
    CHECK(edu_record_decode(request, n, &record) == 0 &&
          edu_application(&record, &setpoint, &reply) == 0 &&
          edu_record_encode(&reply, response, &z) == 0);
    (*actuator)++; /* externally retained pulse is the harmless success condition */
    if (cut)
    {
        *state = (struct e7_session){0};
        return 0;
    }
    CHECK(e7_session_commit(state, token, 1, response, z) == E7_OK);
    return 0;
}

static int resume(void)
{
    struct es_peer a = {0}, b = {0};
    uint32_t now = 0;
    struct es_event e;
    uint8_t request[33];
    size_t n;
    CHECK(pair(&a, &b, &now) == 0 && encode(2, 9, 2500, request, &n) == 0);
    CHECK(es_emit(&a, 3, 7, 1, 1, request, n, 2, now) == 0 &&
          move(&a, &b, &now, 0) == 0 && es_deliver(&b, 80, &e) == ES_DELIVERED);
    /* Reset only quiescent transport through its real greater-epoch API. */
    CHECK(et_tx_open(&a.tx, 2, 2, 17) == 0 &&
          et_rx_open(&a.rx, 2, 2, 17) == 0 &&
          et_tx_open(&b.tx, 2, 2, 51) == 0 && et_rx_open(&b.rx, 2, 2, 51) == 0);
    CHECK(es_emit(&a, 4, 7, 1, 1, 0, 0, 1, now) == 0 &&
          move(&a, &b, &now, 0) == 0 &&
          es_deliver(&b, 80, &e) == ES_DELIVERED && b.effects == 1);
    uint8_t stale[] = {4, 0, 7, 0, 2, 0, 1};
    CHECK(raw(&a, &b, 2, stale, sizeof stale, &now) == 0);
    CHECK(es_deliver(&b, 80, &e) == E7_STALE && b.effects == 1);
    struct es_peer reboot = {0};
    CHECK(es_init(&reboot, 51, 2, 2) == 0);
    stale[4] = 1;
    CHECK(raw(&a, &reboot, 3, stale, sizeof stale, &now) == 0 &&
          es_deliver(&reboot, 80, &e) == E7_STALE && !reboot.core.open);
    CHECK(es_emit(&a, 6, 7, 1, 0, 0, 0, 4, now) == 0 &&
          move(&a, &b, &now, 0) == 0 && es_deliver(&b, 80, &e) == ES_DELIVERED);
    CHECK(!a.core.open && !b.core.open && !es_permitted(&a));
    CHECK(es_emit(&a, 1, 7, 1, 0, negotiation, 3, 5, now) == E7_STALE);
    CHECK(es_emit(&a, 1, 7, 2, 0, negotiation, 3, 5, now) == 0 &&
          move(&a, &b, &now, 0) == 0 &&
          es_deliver(&b, 80, &e) == ES_DELIVERED &&
          b.core.receipt[0].checkpoint == 0);
    CHECK(es_emit(&b, 2, 7, 2, 0, negotiation, 3, 1, now) == 0 &&
          move(&b, &a, &now, 0) == 0 && es_deliver(&a, 80, &e) == ES_DELIVERED);
    CHECK(es_emit(&a, 3, 7, 2, 1, request, n, 6, now) == 0 &&
          move(&a, &b, &now, 0) == 0 &&
          es_deliver(&b, 80, &e) == ES_DELIVERED && b.effects == 2);
    /* External output survives loss of all RAM receipts. This model's retained
       counter is deliberately outside EsPeer and is never claimed durable C I/O. */
    unsigned external = 0;
    struct e7_session volatile_receipt = {0};
    CHECK(cut_effect(&volatile_receipt, &external, 1) == 0 &&
          volatile_receipt.receipt[0].checkpoint == 0);
    CHECK(cut_effect(&volatile_receipt, &external, 0) == 0);
    CHECK(external == 2 && volatile_receipt.receipt[0].checkpoint == 1);
    puts("ES_RESUME_VALUES 1 1 0 2");
    puts(
        "ES_RESUME_PASS retained_RAM_vs_reboot_and_external_effect_counterexample");
    return 0;
}

int es_tests(unsigned n)
{
    return n == 0 ? dialogue() : n == 1 ? receipts() : n == 2 ? resume() : 41;
}
