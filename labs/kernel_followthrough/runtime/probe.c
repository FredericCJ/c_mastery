#include "probe.h"
static struct cvm_cvm machine;
static struct cvm_program program;
#ifdef KF_SCRATCH
static struct kf_verifier_workspace workspace;
#endif
static unsigned callbacks, iterations, fail_callback, loop_count;

static int callback(void *ctx, uint8_t service, int32_t input, int32_t *out)
{
    (void)ctx;
    ++callbacks;
    if (service != 1 || input != 41)
    {
        return -1;
    }
    volatile unsigned tally = 0;
    for (unsigned i = 0; i < loop_count; ++i)
    {
        tally += i;
        ++iterations;
    }
    if (loop_count && tally != (loop_count * (loop_count - 1)) / 2)
    {
        return -1;
    }
    if (fail_callback)
    {
        return -1;
    }
    *out = input + 1;
    return 0;
}

static enum cvm_status load(const uint8_t *p, size_t n)
{
#ifdef KF_SCRATCH
    return kf_load_scratch(&workspace, p, n, &program);
#else
    return cvm_load(p, n, &program);
#endif
}

static enum cvm_status run(struct cvm_value *out)
{
#ifdef KF_SCRATCH
    return kf_run_scratch(&workspace, &machine, &program, out);
#else
    return cvm_run(&machine, &program, out);
#endif
}

int kf_vm_probe(const uint8_t *p, size_t n, struct kf_vm_report *report)
{
    if (!report)
    {
        return 1;
    }
    *report = (struct kf_vm_report){0};
    report->machine_bytes = sizeof machine;
    report->program_bytes = sizeof program;
#ifdef KF_SCRATCH
    report->workspace_bytes = sizeof workspace;
#else
    report->workspace_bytes = 0; /* Baseline verifier owns automatic scratch. */
#endif
    enum cvm_status loaded = load(p, n);
    if (loaded != CVM_OK)
    {
        report->truncated = loaded;
        return n < 19 ? 0 : 2;
    }
    for (unsigned i = 0; i < 6; ++i)
    {
        callbacks = iterations = 0;
        fail_callback = i == 2;
        loop_count = i == 5 ? 256 : 0;
        uint32_t fuel = i == 3   ? 1
                        : i == 4 ? 2
                                 : 32,
                 allowance = i == 1 ? 0 : 1;
        cvm_init(&machine, fuel, 128, allowance);
        machine.host = callback;
        struct cvm_value out = cvm_int(-99);
        enum cvm_status status = run(&out);
        ++report->run_calls;
        report->cases[i] = (struct kf_vm_result){
            status,     out.integer,  callbacks,
            iterations, machine.fuel, machine.host_budget};
        enum cvm_status expected = i == 1               ? CVM_HOST_BUDGET
                                   : i == 2             ? CVM_HOST_ERROR
                                   : (i == 3 || i == 4) ? CVM_FUEL
                                                        : CVM_OK;
        unsigned calls = (i == 1 || i == 3) ? 0 : 1;
        if (status != expected || callbacks != calls ||
            (status == CVM_OK && out.integer != 42) ||
            iterations != (i == 5 ? 256u : 0u))
        {
            return 3;
        }
    }
    report->truncated = load(p, n - 1);
    if (report->truncated == CVM_OK)
    {
        return 4;
    }
    /* VM recursion consumes explicit CvmFrame slots; it is not C recursion. */
    static const uint8_t recursive[] = {
        'C', 'V', 'M', '1', 2,       0, 0, 4,      0,       0, 0, 4,     0,
        4,   0,   0,   0,   OP_CALL, 1, 0, OP_RET, OP_CALL, 1, 0, OP_RET};
    if (load(recursive, sizeof recursive) != CVM_OK)
    {
        return 5;
    }
    cvm_init(&machine, 128, 128, 0);
    struct cvm_value out = cvm_int(0);
    report->recursive = run(&out);
    ++report->run_calls;
    if (report->recursive != CVM_FRAME_LIMIT || machine.depth != 16)
    {
        return 6;
    }
    return 0;
}
