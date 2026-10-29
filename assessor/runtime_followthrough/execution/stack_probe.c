#include "execution.h"
#include <inttypes.h>
#include <stdio.h>
static unsigned depth, maximum, old_depth, old_maximum;
static uintptr_t native_addresses[17];

/* These hooks observe actual instrumented entry/exit, including optimization.
 * Their GCC/Clang attributes are host verification, not target dependencies. */
__attribute__((no_instrument_function)) void
__cyg_profile_func_enter(void *fn, void *caller)
{
    (void)caller;
    if (fn == (void *)vs_step)
    {
        if (++depth > maximum)
            maximum = depth;
    }
    if (fn == (void *)cvm_run)
    {
        if (++old_depth > old_maximum)
            old_maximum = old_depth;
    }
}

__attribute__((no_instrument_function)) void
__cyg_profile_func_exit(void *fn, void *caller)
{
    (void)caller;
    if (fn == (void *)vs_step)
        depth--;
    if (fn == (void *)cvm_run)
        old_depth--;
}

__attribute__((noinline)) static unsigned native(unsigned n)
{
    volatile unsigned char local[96];
    local[0] = (unsigned char)n;
    native_addresses[n] = (uintptr_t)&local[0];
    unsigned result = n ? native(n - 1) : 0;
    return result + local[0];
}
static struct vv_work scratch;
static struct vg_vs static_vm;

static int run(struct vg_vs *s)
{
    struct cvm_program p = {0};
    p.count = 2;
    p.length = 8;
    const unsigned char b[] = {OP_CALL, 1, 0, OP_RET, OP_CALL, 1, 0, OP_RET};
    for (unsigned i = 0; i < 8; i++)
        p.code[i] = b[i];
    p.functions[0] = (struct cvm_function){0, 4, 0, 0};
    p.functions[1] = (struct cvm_function){4, 4, 0, 0};
    struct cvm_value out = cvm_int(91);
    *s = (struct vg_vs){0};
    if (vs_start(s, &p, 0, 100, 0, 0, 0, &scratch) != VS_RUNNING)
        return 1;
    int status = VS_RUNNING;
    for (unsigned i = 0; i < 17 && status == VS_RUNNING; i++)
        status = vs_step(s, &out);
    if (status != CVM_FRAME_LIMIT || s->vm.depth != 16 || out.integer != 91)
        return 1;
    struct cvm_cvm old;
    cvm_init(&old, 100, 0, 0);
    return cvm_run(&old, &p, &out) != CVM_FRAME_LIMIT || old.depth != 16;
}

__attribute__((noinline)) static int automatic(void)
{
    struct vg_vs value = {0};
    return run(&value);
}

int main(void)
{
    if (run(&static_vm) || automatic() || maximum != 1 || old_maximum != 1 ||
        depth || old_depth || native(16) != 136)
        return 1;
    uintptr_t min = native_addresses[0], max = min;
    for (unsigned i = 1; i < 17; i++)
    {
        if (native_addresses[i] < min)
            min = native_addresses[i];
        if (native_addresses[i] > max)
            max = native_addresses[i];
    }
    if (max <= min)
        return 1;
    printf("VS_STACK_BYTES vm=%zu frame=%zu operands=%zu session=%zu "
           "native_span=%" PRIuPTR "\n",
           sizeof(struct cvm_cvm), sizeof(struct cvm_frame),
           sizeof static_vm.vm.stack, sizeof(struct vg_vs), max - min);
    puts("VS_STACK_PASS guest_frames=16 interpreter_c_depth=1 native_depth=17 "
         "storage_modes=2");
    return 0;
}
