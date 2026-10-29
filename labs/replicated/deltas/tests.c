#include "deltas.h"
#include "hashing.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "RP_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)

struct rp_disk
{
    unsigned char stable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE],
        pending[STORE_IMAGE_PAGES][STORE_PAGE_SIZE], dirty[STORE_IMAGE_PAGES];
    unsigned cut, event, mode, dead;
};
static struct rp_disk disks[3];
static store pages[3];
static struct rpd_replica nodes[3];
static unsigned char baseline[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];

static void persist(struct rp_disk *d)
{
    unsigned i;
    for (i = 0; i < STORE_IMAGE_PAGES; ++i)
    {
        if (d->dirty[i])
        {
            memcpy(d->stable[i], d->pending[i], STORE_PAGE_SIZE);
            d->dirty[i] = 0;
        }
    }
}

static int fault(struct rp_disk *d)
{
    if (++d->event != d->cut || !d->cut)
    {
        return 0;
    }
    if (d->mode)
    {
        persist(d);
    }
    d->dead = 1;
    return 1;
}

static int rd(void *c, uint32_t p, void *out)
{
    struct rp_disk *d = c;
    if (d->dead || p >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    memcpy(out, d->dirty[p] ? d->pending[p] : d->stable[p], STORE_PAGE_SIZE);
    return 0;
}

static int wr(void *c, uint32_t p, const void *in)
{
    struct rp_disk *d = c;
    if (d->dead || p >= STORE_IMAGE_PAGES)
    {
        return -1;
    }
    memcpy(d->pending[p], in, STORE_PAGE_SIZE);
    d->dirty[p] = 1;
    return fault(d) ? -1 : 0;
}

static int fl(void *c)
{
    struct rp_disk *d = c;
    if (d->dead || fault(d))
    {
        return -1;
    }
    persist(d);
    return 0;
}

static store_io io(unsigned i)
{
    return (store_io){&disks[i], rd, wr, fl, STORE_IMAGE_PAGES};
}

static int setup(void)
{
    unsigned i;
    memset(disks, 0, sizeof disks);
    for (i = 0; i < 3; ++i)
    {
        CHECK(store_format(io(i)) == STORE_OK);
        CHECK(store_open(&pages[i], io(i)) == STORE_OK);
        CHECK(rpd_create(&pages[i], 9, i, &nodes[i]) == RPD_OK);
    }
    return 0;
}

static int equal(const struct rpd_state *a, const struct rpd_state *b)
{
    unsigned char x[128], y[128];
    return !rpd_encode(a, x) && !rpd_encode(b, y) && !memcmp(x, y, 128);
}

/* Test-only durable boundary injection; this does not simulate 2^64 events. */
static int install(unsigned node, const struct rpd_state *state)
{
    unsigned char bytes[128];
    unsigned i;
    CHECK(rpd_encode(state, bytes) == RPD_OK);
    CHECK(store_begin(&pages[node]) == STORE_OK);
    for (i = 0; i < 2; ++i)
    {
        unsigned char key[2] = {'D', (unsigned char)i};
        CHECK(store_put(&pages[node], key, 2, bytes + 64 * i, 64) == STORE_OK);
    }
    CHECK(store_commit(&pages[node]) == STORE_OK);
    return 0;
}

static int sets(void)
{
    struct rpd_set empty = {0}, a, b, c, old, removed, third, delta, bad;
    unsigned i;
    CHECK(rpd_set_add(&empty, 0, 2, &a, &old) == RPD_OK);
    CHECK(rpd_set_remove(&a, 2, &b, &removed) == RPD_OK);
    CHECK(!rpd_contains(&b, 2));
    CHECK(rpd_set_join(&empty, &removed, &c) == RPD_OK);
    CHECK(rpd_set_join(&c, &old, &a) == RPD_OK);
    CHECK(!rpd_contains(&a, 2));
    CHECK(rpd_set_add(&empty, 2, 2, &third, &delta) == RPD_OK);
    CHECK(rpd_set_join(&b, &delta, &c) == RPD_OK);
    CHECK(rpd_contains(&c, 2));
    CHECK(rpd_set_remove(&c, 2, &a, &removed) == RPD_OK);
    CHECK(rpd_set_join(&a, &old, &b) == RPD_OK);
    CHECK(!rpd_contains(&b, 2));
    bad = b;
    bad.first.adds[2] = bad.first.removed[2] = 0;
    bad.second.adds[2] = bad.second.removed[2] = 0;
    CHECK(rpd_set_join(&bad, &old, &c) == RPD_OK && rpd_contains(&c, 2));
    a = empty;
    for (i = 0; i < 8; ++i)
    {
        CHECK(rpd_set_add(&a, 2, 0, &b, &delta) == RPD_OK);
        a = b;
    }
    CHECK(rpd_set_add(&a, 2, 0, &b, &delta) == RPD_FULL);
    puts("DELTA_TOMBSTONE delayed-old-add-resurrects-after-unsafe-drop=1");
    return 0;
}

static int wire(void)
{
    struct rpd_state a = {0}, out, saved;
    struct rpd_set delta;
    unsigned char b[128], bad[128];
    unsigned i;
    a.uuid = 9;
    a.epoch = 1;
    a.self = 2;
    CHECK(rpd_set_add(&a.set, 2, 1, &out.set, &delta) == RPD_OK);
    a.set = out.set;
    CHECK(rpd_encode(&a, b) == RPD_OK);
    CHECK(rpd_decode(b, &out) == RPD_OK && equal(&a, &out));
    saved = out;
    for (i = 0; i < 1024; ++i)
    {
        memcpy(bad, b, 128);
        bad[i / 8] ^= (unsigned char)(1u << (i % 8));
        CHECK(rpd_decode(bad, &out) == RPD_CORRUPT && equal(&out, &saved));
    }
    memcpy(bad, b, 128);
    bad[50] = 1;
    (void)rph_hash(bad, 96, bad + 96);
    CHECK(rpd_decode(bad, &out) == RPD_CORRUPT);
    puts("DELTA_BITS 1024");
    return 0;
}

static int protocol(void)
{
    struct rpd_packet first, p, old;
    struct rpd_ack ack, oldack;
    uint64_t before;
    unsigned i;
    CHECK(setup() == 0);
    CHECK(rpd_local(&nodes[0], 1, 2) == RPD_OK);
    CHECK(rpd_send(&nodes[0], 1, 1, &first) == RPD_OK && first.full);
    CHECK(rpd_receive(&nodes[1], &first, &ack) == RPD_OK);
    before = nodes[1].state.sequence;
    CHECK(rpd_receive(&nodes[1], &first, &ack) == RPD_OK &&
          nodes[1].state.sequence == before);
    CHECK(rpd_ack(&nodes[0], 1, &ack) == RPD_OK);
    CHECK(rpd_send(&nodes[0], 1, 1, &p) == RPD_EMPTY);
    CHECK(rpd_local(&nodes[0], 1, 1) == RPD_OK);
    CHECK(rpd_send(&nodes[0], 1, 1, &p) == RPD_OK && !p.full && p.start == 1 &&
          p.end == 2);
    old = p;
    old.target = 2;
    CHECK(rpd_receive(&nodes[2], &old, &ack) == RPD_CAUSAL);
    CHECK(rpd_receive(&nodes[1], &p, &ack) == RPD_OK);
    oldack = ack;
    /* Lost acknowledgments and bounded history eviction select a full state. */
    for (i = 0; i < 5; ++i)
    {
        CHECK(rpd_local(&nodes[0], 0, 3) == RPD_OK);
    }
    CHECK(rpd_send(&nodes[0], 1, 1, &p) == RPD_OK && p.full && p.start == 0);
    CHECK(rpd_receive(&nodes[1], &p, &ack) == RPD_OK);
    old = p;
    CHECK(rpd_open(&pages[1], 9, 1, &nodes[1]) == RPD_OK &&
          nodes[1].state.epoch == 2);
    CHECK(rpd_receive(&nodes[1], &old, &ack) == RPD_STALE);
    CHECK(rpd_ack(&nodes[0], 2, &oldack) == RPD_STALE);
    CHECK(rpd_send(&nodes[0], 1, 2, &p) == RPD_OK && p.full);
    CHECK(rpd_receive(&nodes[1], &p, &ack) == RPD_OK);
    CHECK(rpd_ack(&nodes[0], 2, &ack) == RPD_OK);
    old = p;
    oldack = ack;
    CHECK(rpd_open(&pages[0], 9, 0, &nodes[0]) == RPD_OK);
    CHECK(rpd_ack(&nodes[0], 2, &oldack) == RPD_STALE);
    CHECK(rpd_send(&nodes[0], 1, 2, &p) == RPD_OK && p.full);
    CHECK(rpd_receive(&nodes[1], &p, &ack) == RPD_OK);
    before = nodes[1].state.sequence;
    CHECK(rpd_receive(&nodes[1], &old, &ack) == RPD_STALE &&
          nodes[1].state.sequence == before);
    CHECK(rpd_send(&nodes[0], 2, 1, &p) == RPD_OK && p.full);
    CHECK(rpd_receive(&nodes[2], &p, &ack) == RPD_OK);
    CHECK(rpd_contains(&nodes[2].state.set, 1) &&
          rpd_contains(&nodes[2].state.set, 2));
    puts("DELTA_PROTOCOL third-replica lost-ack fallback stale-session "
         "causal-refusal");
    return 0;
}

static int durable(void)
{
    unsigned operation, cut, mode, cases = 0;
    struct rpd_replica source, saved, reopened;
    struct rpd_packet packet;
    struct rpd_ack ack, ack_before;
    int status;
    uint64_t sequence;
    for (operation = 0; operation < 3; ++operation)
    {
        CHECK(setup() == 0);
        CHECK(rpd_local(&nodes[0], 1, 2) == RPD_OK);
        CHECK(rpd_send(&nodes[0], 1, 1, &packet) == RPD_OK);
        source = nodes[1];
        memcpy(baseline, disks[1].stable, sizeof baseline);
        for (cut = 1; cut <= 37; ++cut)
        {
            for (mode = 0; mode < 2; ++mode)
            {
                memset(&disks[1], 0, sizeof disks[1]);
                memcpy(disks[1].stable, baseline, sizeof baseline);
                CHECK(store_open(&pages[1], io(1)) == STORE_OK);
                nodes[1] = source;
                saved = nodes[1];
                memset(&ack, 0xa5, sizeof ack);
                memcpy(&ack_before, &ack, sizeof ack);
                reopened = saved;
                disks[1].event = 0;
                disks[1].cut = cut;
                disks[1].mode = mode;
                if (operation == 0)
                {
                    status = rpd_local(&nodes[1], 1, 3);
                }
                else if (operation == 1)
                {
                    status = rpd_receive(&nodes[1], &packet, &ack);
                }
                else
                {
                    status = rpd_open(&pages[1], 9, 1, &reopened);
                }
                CHECK(status != RPD_OK);
                CHECK(equal(&nodes[1].state, &saved.state));
                CHECK(!memcmp(&ack, &ack_before, sizeof ack));
                CHECK(equal(&reopened.state, &saved.state));
                disks[1].dead = 0;
                disks[1].cut = 0;
                memset(disks[1].dirty, 0, sizeof disks[1].dirty);
                CHECK(store_open(&pages[1], io(1)) == STORE_OK);
                CHECK(rpd_open(&pages[1], 9, 1, &reopened) == RPD_OK);
                sequence = mode && cut >= 36 ? 1 : 0;
                CHECK(reopened.state.sequence ==
                      (operation == 2 ? 0 : sequence));
                CHECK(reopened.state.epoch ==
                      (operation == 2 ? 2 + sequence : 2));
                if (operation < 2)
                {
                    CHECK(rpd_contains(&reopened.state.set,
                                       operation == 0 ? 3 : 2) ==
                          (int)sequence);
                }
                ++cases;
            }
        }
    }
    CHECK(setup() == 0);
    saved = nodes[0];
    CHECK(rpd_create(&pages[0], 9, 0, &reopened) == RPD_STATE);
    CHECK(equal(&saved.state, &nodes[0].state));
    CHECK(store_begin(&pages[0]) == STORE_OK);
    CHECK(rpd_local(&nodes[0], 1, 0) == RPD_STATE && pages[0].active);
    store_abort(&pages[0]);
    /* A copied authority is stale after the actual owner commits progress. */
    saved = nodes[0];
    CHECK(rpd_local(&nodes[0], 1, 0) == RPD_OK);
    CHECK(rpd_local(&saved, 1, 1) == RPD_STATE);
    /* Last sequence is usable once; refusal neither writes nor consumes a tag. */
    nodes[0].state.sequence = UINT64_MAX - 1;
    CHECK(install(0, &nodes[0].state) == 0);
    CHECK(rpd_local(&nodes[0], 0, 3) == RPD_OK &&
          nodes[0].state.sequence == UINT64_MAX);
    saved = nodes[0];
    cut = disks[0].event;
    CHECK(rpd_local(&nodes[0], 1, 1) == RPD_RANGE);
    CHECK(equal(&saved.state, &nodes[0].state) && disks[0].event == cut);
    CHECK(rpd_send(&nodes[0], 1, 1, &packet) == RPD_OK &&
          packet.end == UINT64_MAX);
    /* Duplicate input needs no sequence allocation; new state does. */
    CHECK(rpd_send(&nodes[1], 0, 1, &packet) == RPD_OK);
    CHECK(rpd_receive(&nodes[0], &packet, &ack) == RPD_OK &&
          nodes[0].state.sequence == UINT64_MAX && disks[0].event == cut);
    CHECK(rpd_local(&nodes[1], 1, 3) == RPD_OK);
    CHECK(rpd_send(&nodes[1], 0, 1, &packet) == RPD_OK);
    memset(&ack, 0xa5, sizeof ack);
    memcpy(&ack_before, &ack, sizeof ack);
    CHECK(rpd_receive(&nodes[0], &packet, &ack) == RPD_RANGE);
    CHECK(equal(&saved.state, &nodes[0].state) &&
          !memcmp(&ack, &ack_before, sizeof ack) && disks[0].event == cut);
    nodes[0].state.epoch = UINT64_MAX - 1;
    CHECK(install(0, &nodes[0].state) == 0);
    CHECK(rpd_open(&pages[0], 9, 0, &nodes[0]) == RPD_OK &&
          nodes[0].state.epoch == UINT64_MAX);
    saved = nodes[0];
    reopened = nodes[1];
    source = reopened;
    cut = disks[0].event;
    CHECK(rpd_open(&pages[0], 9, 0, &reopened) == RPD_RANGE);
    CHECK(equal(&source.state, &reopened.state) &&
          equal(&saved.state, &nodes[0].state) && disks[0].event == cut);
    puts(
        "DELTA_LIMITS sequence-last epoch-last stale-authority unchanged-refusal");
    printf("DELTA_CUTS %u\n", cases);
    return 0;
}

static int oracle(void)
{
    struct rpd_packet packets[12];
    struct rpd_ack acknowledgments[12];
    unsigned have_packet[12] = {0}, have_ack[12] = {0};
    char op;
    unsigned a, b, c, i;
    int status;
    uint64_t x[5];
    unsigned char wire[128];
    while (scanf(" %c %u %u %u", &op, &a, &b, &c) == 4)
    {
        CHECK(a < 3);
        for (i = 0; i < 5; ++i)
        {
            x[i] = 0;
        }
        if (op == 'I')
        {
            CHECK(setup() == 0);
            memset(have_packet, 0, sizeof have_packet);
            memset(have_ack, 0, sizeof have_ack);
            status = RPD_OK;
        }
        else if (op == 'L')
        {
            status = rpd_local(&nodes[a], b, c);
        }
        else if (op == 'O')
        {
            status = rpd_open(&pages[a], 9, a, &nodes[a]);
        }
        else if (op == 'S')
        {
            CHECK(b < 3 && c < 12);
            status = rpd_send(&nodes[a], b, nodes[b].state.epoch, &packets[c]);
            if (!status)
            {
                have_packet[c] = 1;
                x[0] = packets[c].full;
                x[1] = packets[c].start;
                x[2] = packets[c].end;
                x[3] = packets[c].sender_epoch;
                x[4] = packets[c].target_epoch;
            }
        }
        else if (op == 'D')
        {
            CHECK(b < 12 && have_packet[b]);
            status = rpd_receive(&nodes[a], &packets[b], &acknowledgments[b]);
            if (!status)
            {
                have_ack[b] = 1;
                x[0] = acknowledgments[b].from;
                x[1] = acknowledgments[b].to;
                x[2] = acknowledgments[b].from_epoch;
                x[3] = acknowledgments[b].to_epoch;
                x[4] = acknowledgments[b].upto;
            }
        }
        else if (op == 'K')
        {
            CHECK(b < 12 && have_ack[b]);
            status =
                rpd_ack(&nodes[a], nodes[acknowledgments[b].from].state.epoch,
                        &acknowledgments[b]);
        }
        else
        {
            return 2;
        }
        printf("%d", status);
        for (i = 0; i < 5; ++i)
        {
            printf(" %" PRIu64, x[i]);
        }
        for (a = 0; a < 3; ++a)
        {
            CHECK(rpd_encode(&nodes[a].state, wire) == RPD_OK);
            putchar(' ');
            for (i = 0; i < 128; ++i)
            {
                printf("%02x", wire[i]);
            }
        }
        putchar('\n');
    }
    return 0;
}

static int metrics(void)
{
    printf(
        "{\"set_size\":%zu,\"state_size\":%zu,\"replica_size\":%zu,\"packet_"
        "size\":%zu,\"ack_size\":%zu,\"history_capacity\":4,\"tag_capacity\":"
        "24,\"snapshot_bytes\":128}\n",
        sizeof(struct rpd_set), sizeof(struct rpd_state),
        sizeof(struct rpd_replica), sizeof(struct rpd_packet),
        sizeof(struct rpd_ack));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "delta_sets"))
    {
        return sets();
    }
    if (!strcmp(argv[1], "delta_wire"))
    {
        return wire();
    }
    if (!strcmp(argv[1], "delta_protocol"))
    {
        return protocol();
    }
    if (!strcmp(argv[1], "delta_durable"))
    {
        return durable();
    }
    if (!strcmp(argv[1], "oracle"))
    {
        return oracle();
    }
    if (!strcmp(argv[1], "metrics"))
    {
        return metrics();
    }
    return 2;
}
