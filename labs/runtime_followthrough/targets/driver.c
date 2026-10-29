#include "fixture.h"
#include "probe.h"
#include <limits.h>
#include <stdio.h>
_Static_assert(UINT_MAX == UINT32_MAX,
               "explicit 32-bit unsigned diagnostic profile");
static volatile uint32_t initialized = UINT32_C(0x12345678);
static volatile uint32_t zeroed;

static void signed_value(int32_t value)
{
    if (value < 0)
    {
        printf("-%u", (unsigned)(-(int64_t)value));
    }
    else
    {
        printf("%u", (unsigned)value);
    }
}

int main(void)
{
    if (initialized != UINT32_C(0x12345678) || zeroed != 0)
    {
        puts("VX_STARTUP_FAIL");
        return 1;
    }
    zeroed = initialized;
    puts("VX_STARTUP_PASS data=305419896 bss=0");
    struct kf_vm_report r;
    volatile const uint8_t *wire = kf_vm_bytes;
    printf("VX_BYTES");
    for (unsigned i = 0; i < sizeof kf_vm_bytes; i++)
    {
        printf(":%u", (unsigned)wire[i]);
    }
    puts("");
    int status = kf_vm_probe(kf_vm_bytes, sizeof kf_vm_bytes, &r);
    if (status)
    {
        printf("VX_FAIL:%u\n", (unsigned)status);
        return 1;
    }
    for (unsigned i = 0; i < 6; i++)
    {
        struct kf_vm_result *c = &r.cases[i];
        printf("KF_VM:%u:%u:", i, (unsigned)c->status);
        signed_value(c->value);
        printf(":%u:%u:%u:%u\n", c->calls, c->loop_iterations,
               (unsigned)c->fuel_left, (unsigned)c->host_left);
    }
    printf("KF_VM_TAIL:%u:%u:%u:%u:%u:%u\n", (unsigned)r.truncated,
           (unsigned)r.recursive, r.run_calls, (unsigned)r.machine_bytes,
           (unsigned)r.program_bytes, (unsigned)r.workspace_bytes);
    puts("VX_PROBE_PASS same_core_six_rows");
    return 0;
}
