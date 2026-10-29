#include "cvm.h"
#include "medium.h"
#include "workflow.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                            \
    do                                                                      \
    {                                                                       \
        if (!(x))                                                           \
        {                                                                   \
            fprintf(stderr, "SW_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                       \
        }                                                                   \
    } while (0)
static struct d_medium disks[3], snapshots[3];
static store databases[3];
static struct vg_vh left, right;
static struct sw_flow flow;

static int32_t read_value(struct vg_vh *h)
{
    int32_t v = INT32_MIN;
    if (vh_host(h, 2, 0, &v))
        return INT32_MIN;
    return v;
}

static int open_all(void)
{
    for (unsigned i = 0; i < 3; i++)
    {
        dm_reboot(disks + i);
        CHECK(store_open(databases + i, dm_io(disks + i)) == STORE_OK);
        CHECK(store_check(databases + i) == STORE_OK);
    }
    left = (struct vg_vh){databases + 1, 14, VH_VERSIONED5, 0, 1};
    right = (struct vg_vh){databases + 2, 14, VH_VERSIONED5, 0, 1};
    return 0;
}

static int init_amount(unsigned decline, int32_t amount)
{
    for (unsigned i = 0; i < 3; i++)
    {
        dm_init(disks + i);
        CHECK(store_format(dm_io(disks + i)) == STORE_OK);
    }
    CHECK(open_all() == 0);
    int32_t out;
    CHECK(vh_host(&left, 1, 10, &out) == STORE_OK);
    CHECK(vh_host(&right, 1, 20, &out) == STORE_OK);
    CHECK(sw_create(&flow, databases, &left, &right, amount, decline) ==
          SW_PROGRESS);
    for (unsigned i = 0; i < 3; i++)
    {
        CHECK(store_checkpoint(databases + i) == STORE_OK);
        CHECK(dm_arm(disks + i, 0, 0, 0, 0, 0) == 0);
    }
    memcpy(snapshots, disks, sizeof disks);
    return 0;
}

static int init(unsigned decline)
{
    return init_amount(decline, 4);
}

static int restore(void)
{
    memcpy(disks, snapshots, sizeof disks);
    CHECK(open_all() == 0);
    CHECK(sw_open(&flow, databases, &left, &right) == SW_PROGRESS);
    return 0;
}

static int arm_all(unsigned target, unsigned cut, unsigned mode)
{
    for (unsigned i = 0; i < 3; i++)
        CHECK(dm_arm(disks + i, i == target ? cut : 0, mode, 0, 0, 0) == 0);
    return 0;
}

static int finish(unsigned decline)
{
    for (unsigned step = 0; step < 5; step++)
    {
        CHECK(arm_all(3, 0, 0) == 0);
        int r = sw_step(&flow);
        if (r == SW_DONE || r == SW_COMPENSATED)
        {
            CHECK(r == (decline ? SW_COMPENSATED : SW_DONE));
            CHECK(read_value(&left) == (decline ? 10 : 6) &&
                  read_value(&right) == (decline ? 20 : 24));
            for (unsigned i = 0; i < 3; i++)
                CHECK(store_check(databases + i) == STORE_OK);
            return 0;
        }
        CHECK(r == SW_PROGRESS);
    }
    CHECK(0);
}

static int ordinary(void)
{
    for (unsigned decline = 0; decline < 2; decline++)
    {
        CHECK(init(decline) == 0);
        CHECK(finish(decline) == 0);
        struct sw_flow unused = flow;
        CHECK(sw_create(&unused, databases, &left, &right, 4, decline) ==
              SW_CONFLICT);
        CHECK(open_all() == 0);
        CHECK(sw_open(&flow, databases, &left, &right) == SW_PROGRESS);
        CHECK(finish(decline) == 0);
    }
    puts("SW ordinary validated transfer6/24 compensation10/20 "
         "durable-intent-and-decision");
    return 0;
}

static int cuts(void)
{
    unsigned histories = 0;
    for (unsigned decline = 0; decline < 2; decline++)
    {
        CHECK(init(decline) == 0);
        unsigned steps = decline ? 3 : 2;
        for (unsigned step = 0; step < steps; step++)
        {
            CHECK(restore() == 0);
            for (unsigned j = 0; j < step; j++)
            {
                CHECK(arm_all(3, 0, 0) == 0);
                CHECK(sw_step(&flow) == SW_PROGRESS);
            }
            CHECK(arm_all(3, 0, 0) == 0);
            int baseline = sw_step(&flow);
            CHECK(baseline == SW_PROGRESS ||
                  baseline == (decline ? SW_COMPENSATED : SW_DONE));
            unsigned events[3];
            for (unsigned i = 0; i < 3; i++)
                events[i] = disks[i].event;
            for (unsigned device = 0; device < 3; device++)
                for (unsigned mode = 0; mode < 2; mode++)
                    for (unsigned cut = 1; cut <= events[device]; cut++)
                    {
                        CHECK(restore() == 0);
                        for (unsigned j = 0; j < step; j++)
                        {
                            CHECK(arm_all(3, 0, 0) == 0);
                            CHECK(sw_step(&flow) == SW_PROGRESS);
                        }
                        CHECK(arm_all(device, cut, mode ? DM_ALL : DM_NONE) ==
                              0);
                        CHECK(sw_step(&flow) == SW_RECOVER);
                        CHECK(flow.poisoned);
                        CHECK(open_all() == 0);
                        CHECK(sw_open(&flow, databases, &left, &right) ==
                              SW_PROGRESS);
                        CHECK(finish(decline) == 0);
                        histories++;
                    }
        }
    }
    printf("SW cuts validated histories=%u "
           "participant-and-coordinator-all-observed-boundaries\n",
           histories);
    return 0;
}

static int create_cut(void)
{
    unsigned histories = 0;
    for (unsigned mode = 0; mode < 2; mode++)
        for (unsigned cut = 1; cut <= 37; cut++)
        {
            for (unsigned i = 0; i < 3; i++)
            {
                dm_init(disks + i);
                CHECK(store_format(dm_io(disks + i)) == STORE_OK);
            }
            CHECK(open_all() == 0);
            int32_t out;
            CHECK(vh_host(&left, 1, 10, &out) == STORE_OK);
            CHECK(vh_host(&right, 1, 20, &out) == STORE_OK);
            CHECK(arm_all(0, cut, mode ? DM_ALL : DM_NONE) == 0);
            struct sw_flow before = {0};
            flow = before;
            CHECK(sw_create(&flow, databases, &left, &right, 4, 0) ==
                  SW_RECOVER);
            CHECK(!memcmp(&flow, &before, sizeof flow));
            CHECK(read_value(&left) == 10 && read_value(&right) == 20);
            CHECK(open_all() == 0);
            uint8_t bytes[64];
            size_t n;
            int r =
                store_get(databases, "workflow", 8, bytes, sizeof bytes, &n);
            CHECK(r == (mode && cut >= 36 ? STORE_OK : STORE_NOT_FOUND));
            histories++;
        }
    printf("SW create validated cuts=%u no-effect-before-durable-intent\n",
           histories);
    return 0;
}

static int uncertain(void)
{
    CHECK(init(0) == 0);
    CHECK(sw_step(&flow) == SW_PROGRESS);
    CHECK(arm_all(2, 37, DM_ALL) == 0);
    CHECK(sw_step(&flow) == SW_RECOVER);
    CHECK(flow.poisoned);
    CHECK(open_all() == 0);
    CHECK(sw_open(&flow, databases, &left, &right) == SW_PROGRESS);
    CHECK(flow.decline == 0 && flow.phase == SW_DEBITED);
    CHECK(read_value(&right) == 24);
    CHECK(finish(0) == 0);
    puts("SW uncertain validated persisted-credit-never-auto-compensated");
    return 0;
}

static int last_host, calls;

static int observed(void *ctx, uint8_t service, int32_t input, int32_t *out)
{
    calls++;
    last_host = vh_host(ctx, service, input, out);
    return last_host;
}

static enum cvm_status call(struct vg_vh *h, unsigned service, unsigned budget,
                            int32_t value, int32_t *out)
{
    struct cvm_cvm vm;
    struct cvm_program p = {0};
    p.count = 1;
    p.length = 8;
    p.functions[0] = (struct cvm_function){0, 8, 0, 0};
    p.code[0] = OP_INT;
    uint32_t u = (uint32_t)value;
    for (unsigned i = 0; i < 4; i++)
        p.code[i + 1] = (uint8_t)(u >> (8 * i));
    p.code[5] = OP_HOST;
    p.code[6] = (uint8_t)service;
    p.code[7] = OP_RET;
    cvm_init(&vm, 16, 0, budget);
    vm.host = observed;
    vm.host_context = h;
    struct cvm_value outv = cvm_int(*out);
    enum cvm_status r = cvm_run(&vm, &p, &outv);
    if (r == CVM_OK)
        *out = outv.integer;
    return r;
}

static void capacity_key(char key[8], unsigned value)
{
    key[0] = 'a';
    key[7] = 0;
    for (unsigned n = 0; n < 6; n++)
    {
        key[6 - n] = (char)('0' + value % 10);
        value /= 10;
    }
}

static int capacity(void)
{
    dm_init(disks);
    CHECK(store_format(dm_io(disks)) == STORE_OK);
    dm_reboot(disks);
    CHECK(store_open(databases, dm_io(disks)) == STORE_OK);
    struct vg_vh owner = {databases, UINT32_MAX, VH_LEGACY4, 0, 0};
    unsigned keys = 0;
    for (unsigned i = 0; i < 512; i++)
    {
        CHECK(dm_arm(disks, 0, 0, 0, 0, 0) == 0);
        CHECK(store_begin(databases) == STORE_OK);
        char key[8];
        capacity_key(key, i);
        int r = store_put(databases, key, 7, "x", 1);
        if (r == STORE_FULL)
        {
            store_abort(databases);
            break;
        }
        CHECK(r == STORE_OK && store_commit(databases) == STORE_OK);
        keys++;
    }
    CHECK(keys > 0 && keys < 512);
    CHECK(dm_arm(disks, 0, 0, 0, 0, 0) == 0);
    CHECK(store_checkpoint(databases) == STORE_OK);
    CHECK(dm_arm(disks, 0, 0, 0, 0, 0) == 0);
    int32_t out = 91;
    calls = 0;
    last_host = -99;
    CHECK(call(&owner, 1, 0, 7, &out) == CVM_HOST_BUDGET && out == 91 &&
          calls == 0 && last_host == -99 && disks[0].event == 0);
    CHECK(call(&owner, 31, 1, 7, &out) == CVM_HOST_ERROR && out == 91 &&
          calls == 1 && last_host == STORE_INVALID && disks[0].event == 0);
    CHECK(call(&owner, 1, 1, 7, &out) == CVM_HOST_ERROR && out == 91 &&
          calls == 2 && last_host == STORE_FULL && disks[0].event == 0 &&
          !databases[0].active && !databases[0].poisoned);
    CHECK(store_check(databases) == STORE_OK);
    for (unsigned i = 0; i < keys; i++)
    {
        char key[8], value = 0;
        size_t n = 0;
        capacity_key(key, i);
        CHECK(store_get(databases, key, 7, &value, 1, &n) == STORE_OK &&
              n == 1 && value == 'x');
    }
    printf("SW capacity validated keys=%u host-budget-before-callback "
           "unknown-ID-before-IO actual-store-FULL\n",
           keys);
    return 0;
}

static int oracle(void)
{
    const int32_t amounts[] = {1, 4, 10};
    for (unsigned d = 0; d < 2; d++)
        for (unsigned a = 0; a < 3; a++)
        {
            CHECK(init_amount(d, amounts[a]) == 0);
            for (unsigned step = 0; step < (d ? 3u : 2u); step++)
            {
                CHECK(arm_all(3, 0, 0) == 0);
                int r = sw_step(&flow);
                CHECK(r == SW_PROGRESS || r == SW_DONE || r == SW_COMPENSATED);
                uint8_t p[64];
                size_t n;
                CHECK(store_get(databases, "workflow", 8, p, sizeof p, &n) ==
                          STORE_OK &&
                      n == 7);
                printf("%u,%d,%u,%u,%d,%d,%d,", d, amounts[a], step, flow.phase,
                       r, read_value(&left), read_value(&right));
                for (size_t j = 0; j < n; j++)
                    printf("%02x", p[j]);
                puts("");
                CHECK(open_all() == 0);
                CHECK(sw_open(&flow, databases, &left, &right) == SW_PROGRESS);
            }
        }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    return !strcmp(argv[1], "ordinary")    ? ordinary()
           : !strcmp(argv[1], "cuts")      ? cuts()
           : !strcmp(argv[1], "create")    ? create_cut()
           : !strcmp(argv[1], "uncertain") ? uncertain()
           : !strcmp(argv[1], "capacity")  ? capacity()
           : !strcmp(argv[1], "oracle")    ? oracle()
                                           : 2;
}
