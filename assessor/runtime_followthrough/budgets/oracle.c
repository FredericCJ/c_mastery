#include "budgets.h"
#include "fixtures.h"
#include <stdio.h>
static unsigned effects;

static int host(void *ctx, uint8_t id, int32_t x, int32_t *out)
{
    (void)ctx;
    if (id != 1)
        return 1;
    effects++;
    *out = x + 11;
    return 0;
}

int main(void)
{
    unsigned f, g, h, status, uf, ug, uh, allocations, collections,
        count = 0, expected_effects;
    int value;
    struct cvm_program p = vl_composed();
    while (scanf("%u%u%u%u%u%u%u%u%u%u%d", &f, &g, &h, &status, &uf, &ug, &uh,
                 &allocations, &collections, &expected_effects, &value) == 11)
    {
        struct cvm_cvm vm;
        struct cvm_value out = cvm_int(91);
        struct vl_report r;
        effects = 0;
        cvm_init(&vm, f, g, h);
        vm.host = host;
        int s = vl_run(&vm, &p, &out, &r);
        if (s != (int)status || r.status != s || r.fuel != uf || r.gc != ug ||
            r.host != uh || r.allocations != allocations ||
            r.collections != collections || effects != expected_effects ||
            out.integer != value)
        {
            fprintf(
                stderr,
                "VL_ORACLE_REFUSE budgets=%u,%u,%u status=%d units=%u,%u,%u\n",
                f, g, h, s, r.fuel, r.gc, r.host);
            return 1;
        }
        count++;
    }
    if (!feof(stdin) || count != 693)
        return 2;
    printf("VL_ORACLE_PASS cases=%u independent_order_and_charge\n", count);
    return 0;
}
