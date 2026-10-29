#include "checkpoint_writer.h"
#include <limits.h>
#ifndef CP_MUTANT
#define CP_MUTANT 0
#endif
struct cpw_result cpw_raw(long value)
{
    if (value > 0)
    {
        return (struct cpw_result){CPW_PROGRESS, (size_t)value, 0};
    }
    if (value == 0)
    {
        return (struct cpw_result){CPW_RETRY, 0, 0};
    }
    if (value == -4 || value == -11)
    {
        return (struct cpw_result){CPW_RETRY, 0, (int)-value};
    }
    if (value < -4095)
    {
        return (struct cpw_result){CPW_ERROR, 0, 4096};
    }
    return (struct cpw_result){CPW_ERROR, 0, (int)-value};
}

struct cpw_result cpw_libc(long value, int error)
{
    if (value != -1)
    {
        return cpw_raw(value);
    }
    if (error <= 0 || error > 4095)
    {
        return (struct cpw_result){CPW_ERROR, 0, 4096};
    }
    return cpw_raw(-(long)error);
}

struct cpw_result cpw_script(long value)
{
    if (value == -2)
    {
        return (struct cpw_result){CPW_RETRY, 0, 1002};
    }
    return value < 0 ? (struct cpw_result){CPW_ERROR, 0, 1001} : cpw_raw(value);
}

enum cpw_status cpw_begin(struct cpw_transfer *t, const unsigned char *data,
                          size_t n, unsigned operation)
{
    if (!t || (n && !data) || n > (size_t)LONG_MAX)
    {
        return CPW_INVALID;
    }
    *t = (struct cpw_transfer){
        data, n, 0, n ? CPW_PENDING : CPW_DONE, {operation, 0, 0, 0}};
    return t->status;
}

enum cpw_status cpw_step(struct cpw_transfer *t, cpw_write_fn write,
                         void *context)
{
    if (!t || !write)
    {
        return CPW_INVALID;
    }
    if (t->status != CPW_PENDING)
    {
        return t->status;
    }
    size_t remaining = t->length - t->accepted;
    struct cpw_result result = write(context, t->data + t->accepted, remaining);
    if (result.kind == CPW_PROGRESS && result.accepted &&
        result.accepted <= remaining && result.cause == 0)
    {
        t->accepted += CP_MUTANT == 19 ? remaining : result.accepted;
        if (t->accepted == t->length)
        {
            t->status = CPW_DONE;
        }
    }
    else if (result.kind == CPW_RETRY && !result.accepted)
    {
        if (CP_MUTANT == 20 && t->accepted < t->length)
        {
            ++t->accepted;
        }
    }
    else
    {
        t->status = CPW_FAILED;
        if (!t->error.primary)
        {
            t->error.primary =
                result.kind == CPW_ERROR && !result.accepted && result.cause > 0
                    ? result.cause
                    : 4096;
        }
    }
    t->error.accepted = t->accepted;
    return t->status;
}

enum cpw_status cpw_recover(struct cpw_transfer *t)
{
    if (!t || t->status != CPW_FAILED)
    {
        return CPW_INVALID;
    }
    t->status = t->accepted == t->length ? CPW_DONE : CPW_PENDING;
    return t->status;
}

void cpw_cleanup(struct cpw_transfer *t, int cause)
{
    if (t && cause > 0 && !t->error.cleanup)
    {
        t->error.cleanup = cause;
    }
    if (CP_MUTANT == 22 && t && cause > 0)
    {
        t->error.primary = cause;
    }
}
