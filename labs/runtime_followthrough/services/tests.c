#include "cvm.h"
#include "medium.h"
#include "services.h"
#include "store_host.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("VH_CHECK %u %s\n", (unsigned)__LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)
static struct vh_medium disk, snapshot;
static store database;
static struct vg_vh host;

static struct cvm_program program(uint8_t service, int32_t n, int boolean)
{
    struct cvm_program p = {0};
    unsigned i = 0;
    uint32_t u = (uint32_t)n;
    if (boolean)
    {
        p.code[i++] = OP_TRUE;
    }
    else
    {
        p.code[i++] = OP_INT;
        for (unsigned j = 0; j < 4; j++)
        {
            p.code[i++] = (uint8_t)u;
            u >>= 8;
        }
    }
    p.code[i++] = OP_HOST;
    p.code[i++] = service;
    p.code[i++] = OP_RET;
    p.count = 1;
    p.length = (uint16_t)i;
    p.functions[0] = (struct cvm_function){0, (uint16_t)i, 0, 0};
    return p;
}

static int call(struct vg_vh *h, uint8_t service, int32_t n, unsigned budget,
                int boolean, struct cvm_value *out)
{
    struct cvm_cvm vm;
    struct cvm_program p = program(service, n, boolean);
    cvm_init(&vm, 3, 0, budget);
    vm.host = vh_host;
    vm.host_context = h;
    return cvm_run(&vm, &p, out);
}

static int fresh(unsigned mode)
{
    memset(&disk, 0, sizeof disk);
    CHECK(store_format(vh_medium_io(&disk)) == STORE_OK);
    CHECK(store_open(&database, vh_medium_io(&disk)) == STORE_OK);
    host = (struct vg_vh){&database, 14, mode, 0, 0};
    return 0;
}

static int raw(const uint8_t *p, size_t n)
{
    CHECK(store_begin(&database) == STORE_OK);
    CHECK(store_put(&database, "vm-result", 9, p, n) == STORE_OK);
    CHECK(store_commit(&database) == STORE_OK);
    return 0;
}

static int recover(void)
{
    vh_medium_restart(&disk);
    CHECK(store_open(&database, vh_medium_io(&disk)) == STORE_OK);
    CHECK(store_check(&database) == STORE_OK);
    return 0;
}

static int policies(void)
{
    CHECK(fresh(VH_LEGACY4) == 0);
    struct cvm_value out = cvm_int(91);
    unsigned old_events = disk.event;
    struct cvm_cvm absent;
    struct cvm_program absent_program = program(1, 7, 0);
    cvm_init(&absent, 3, 0, 1);
    CHECK(cvm_run(&absent, &absent_program, &out) == CVM_HOST_ERROR &&
          out.integer == 91 && disk.event == old_events);
    struct vg_vh unknown = host;
    unknown.capabilities = UINT32_C(1) << 31;
    CHECK(call(&unknown, 31, 7, 1, 0, &out) == CVM_HOST_ERROR &&
          out.integer == 91 && disk.event == old_events);
    struct vg_vh writer = host, reader = host;
    writer.capabilities = 2;
    reader.capabilities = 4;
    CHECK(call(&reader, 1, 7, 1, 0, &out) == CVM_HOST_ERROR &&
          out.integer == 91 && disk.event == old_events);
    CHECK(call(&writer, 1, 7, 0, 0, &out) == CVM_HOST_BUDGET &&
          out.integer == 91 && disk.event == old_events);
    CHECK(call(&writer, 1, 7, 1, 1, &out) == CVM_TYPE && out.integer == 91 &&
          disk.event == old_events);
    CHECK(call(&writer, 31, 7, 1, 0, &out) == CVM_HOST_ERROR &&
          out.integer == 91 && disk.event == old_events);
    CHECK(call(&writer, 255, 7, 1, 0, &out) == CVM_HOST_ERROR &&
          out.integer == 91 && disk.event == old_events);
    CHECK(call(&writer, 1, 7, 1, 0, &out) == CVM_OK && out.integer == 7);
    out = cvm_int(91);
    uint64_t reads = database.reads, hits = database.cache_hits;
    CHECK(call(&writer, 2, 0, 1, 0, &out) == CVM_HOST_ERROR &&
          out.integer == 91);
    CHECK(database.reads == reads && database.cache_hits == hits);
    CHECK(call(&reader, 2, 0, 1, 0, &out) == CVM_OK && out.integer == 7);
    CHECK(store_begin(&database) == STORE_OK);
    old_events = disk.event;
    out = cvm_int(91);
    CHECK(call(&writer, 1, 8, 1, 0, &out) == CVM_HOST_ERROR &&
          out.integer == 91 && disk.event == old_events && database.active);
    store_abort(&database);
    puts("VH_POLICY_PASS two_VM_capabilities_tag_budget_no_backend_effect");
    return 0;
}

static int codecs(void)
{
    CHECK(fresh(VH_LEGACY4) == 0);
    struct cvm_value out;
    const int32_t values[] = {0, 1, -1, INT32_MIN, INT32_MAX, -91, 16909060};
    for (unsigned i = 0; i < sizeof values / sizeof values[0]; i++)
    {
        CHECK(call(&host, 1, values[i], 1, 0, &out) == CVM_OK &&
              out.integer == values[i]);
        uint8_t bytes[64];
        size_t n = 0;
        CHECK(store_get(&database, "vm-result", 9, bytes, sizeof bytes, &n) ==
                  STORE_OK &&
              n == 4);
        uint32_t u = (uint32_t)values[i];
        for (unsigned j = 0; j < 4; j++)
        {
            CHECK(bytes[j] == (uint8_t)u);
            u >>= 8;
        }
        CHECK(recover() == 0);
        CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_OK &&
              out.integer == values[i]);
    }
    uint8_t bad[6] = {1, 2, 3, 4, 5, 6};
    CHECK(raw(bad, 3) == 0);
    out = cvm_int(91);
    CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    host.mode = VH_VERSIONED5;
    CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    bad[0] = 2;
    CHECK(raw(bad, 5) == 0);
    CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    host.mode = VH_LEGACY4;
    CHECK(call(&host, 1, INT32_MIN, 1, 0, &out) == CVM_OK);
    host.mode = VH_VERSIONED5;
    out = cvm_int(91);
    CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    host.allow_legacy = 1;
    CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_OK && out.integer == INT32_MIN);
    int32_t n = 91;
    CHECK(vh_migrate(&host, &n) == STORE_OK && n == INT32_MIN);
    CHECK(recover() == 0);
    host.allow_legacy = 0;
    CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_OK && out.integer == INT32_MIN);
    host.allow_legacy = 1;
    unsigned events = disk.event;
    CHECK(vh_migrate(&host, &n) == STORE_OK && n == INT32_MIN &&
          disk.event == events);
    for (unsigned i = 0; i < sizeof values / sizeof values[0]; i++)
    {
        CHECK(call(&host, 1, values[i], 1, 0, &out) == CVM_OK);
        CHECK(recover() == 0);
        CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_OK &&
              out.integer == values[i]);
    }
    puts("VH_CODEC_PASS actual_legacy4_versioned5_extrema_migration");
    return 0;
}

static int uncertain_assignment(void)
{
    CHECK(fresh(VH_VERSIONED5) == 0);
    struct cvm_value out = cvm_int(91);
    CHECK(call(&host, 1, 10, 1, 0, &out) == CVM_OK);
    CHECK(store_checkpoint(&database) == STORE_OK);
    snapshot = disk;
    /* Invalid marker+flush+32images+flush+commit marker+flush =37 events. */
    for (unsigned persist = 0; persist < 2; persist++)
    {
        disk = snapshot;
        CHECK(recover() == 0);
        disk.cut = persist ? 37 : 1;
        disk.persist_on_failure = persist;
        out = cvm_int(91);
        CHECK(call(&host, 1, 20, 1, 0, &out) == CVM_HOST_ERROR &&
              out.integer == 91);
        CHECK(database.poisoned);
        CHECK(recover() == 0);
        CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_OK &&
              out.integer == (persist ? 20 : 10));
        if (out.integer != 20)
        {
            CHECK(call(&host, 1, 20, 1, 0, &out) == CVM_OK);
        }
        CHECK(out.integer == 20);
    }
    puts("VH_UNCERTAIN_PASS same_HOST_ERROR_old10_or_committed20");
    return 0;
}

static int receipts(void)
{
    CHECK(fresh(VH_VERSIONED5) == 0);
    struct cvm_value out;
    CHECK(call(&host, 1, 10, 1, 0, &out) == CVM_OK);
    host.operation_id = 1;
    CHECK(call(&host, 3, 5, 1, 0, &out) == CVM_OK && out.integer == 15);
    uint8_t bytes[64];
    size_t n = 0;
    CHECK(store_get(&database, "vm-operation", 12, bytes, sizeof bytes, &n) ==
              STORE_OK &&
          n == 17);
    unsigned events = disk.event;
    CHECK(call(&host, 3, 5, 1, 0, &out) == CVM_OK && out.integer == 15 &&
          disk.event == events);
    out = cvm_int(91);
    CHECK(call(&host, 3, 6, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    host.operation_id = 3;
    CHECK(call(&host, 3, 5, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    host.operation_id = 2;
    CHECK(call(&host, 3, -4, 1, 0, &out) == CVM_OK && out.integer == 11);
    host.operation_id = 1;
    out = cvm_int(91);
    CHECK(call(&host, 3, 5, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    host.operation_id = 3;
    CHECK(call(&host, 3, INT32_MAX, 1, 0, &out) == CVM_HOST_ERROR &&
          out.integer == 91);
    host.operation_id = 0;
    CHECK(call(&host, 3, 0, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    /* Controlled valid last-operation model: reject wrap, retain exact retry. */
    CHECK(call(&host, 1, 15, 1, 0, &out) == CVM_OK);
    uint8_t last[17] = {1};
    for (unsigned i = 1; i < 9; i++)
    {
        last[i] = 255;
    }
    last[9] = 1;
    last[13] = 15;
    CHECK(store_begin(&database) == STORE_OK);
    CHECK(store_put(&database, "vm-operation", 12, last, sizeof last) ==
          STORE_OK);
    CHECK(store_commit(&database) == STORE_OK);
    host.operation_id = UINT64_MAX;
    CHECK(call(&host, 3, 1, 1, 0, &out) == CVM_OK && out.integer == 15);
    host.operation_id = UINT64_MAX - 1;
    out = cvm_int(91);
    CHECK(call(&host, 3, 1, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    host.operation_id = 0;
    CHECK(call(&host, 3, 1, 1, 0, &out) == CVM_HOST_ERROR && out.integer == 91);
    puts("VH_RECEIPT_PASS exact_latest_duplicate_payload_sequence_overflow");
    return 0;
}

static int crash_increment(void)
{
    CHECK(fresh(VH_VERSIONED5) == 0);
    struct cvm_value out;
    CHECK(call(&host, 1, 10, 1, 0, &out) == CVM_OK);
    CHECK(store_checkpoint(&database) == STORE_OK);
    snapshot = disk;
    host.operation_id = 1;
    unsigned old = 0, newer = 0;
    for (unsigned mode = 0; mode < 2; mode++)
    {
        for (unsigned cut = 1; cut <= 37; cut++)
        {
            disk = snapshot;
            CHECK(recover() == 0);
            disk.cut = cut;
            disk.persist_on_failure = mode;
            out = cvm_int(91);
            CHECK(call(&host, 3, 5, 1, 0, &out) == CVM_HOST_ERROR &&
                  out.integer == 91);
            CHECK(recover() == 0);
            CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_OK);
            uint8_t receipt[64];
            size_t n = 0;
            int r = store_get(&database, "vm-operation", 12, receipt,
                              sizeof receipt, &n);
            int committed = mode && cut >= 36;
            CHECK(out.integer == (committed ? 15 : 10));
            CHECK((r == STORE_OK && n == 17) == committed);
            if (committed)
            {
                newer++;
            }
            else
            {
                old++;
            }
            CHECK(call(&host, 3, 5, 1, 0, &out) == CVM_OK && out.integer == 15);
            CHECK(recover() == 0);
            CHECK(call(&host, 2, 0, 1, 0, &out) == CVM_OK && out.integer == 15);
        }
    }
    CHECK(old == 72 && newer == 2);
    puts(
        "VH_CRASH_PASS cuts=74 old=72 new=2 replay_exactly_once_within_protocol");
    return 0;
}

int main(void)
{
    if (policies() || codecs() || uncertain_assignment() || receipts() ||
        crash_increment())
    {
        return 1;
    }
    puts("VH_PASS actual_VM_store_capabilities_versioned_receipts");
    return 0;
}
