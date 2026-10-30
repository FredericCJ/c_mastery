/*
 * Compiler experiment: functions with known source-level stack arrays, built
 * at several optimization levels so the generated frames can be compared with
 * the source byte counts. Record each .su frame size beside the source byte
 * count of its arrays, per optimization level.
 *
 * The arrays are volatile to keep the objects concrete. main only checks that
 * every variant returns its expected value.
 */
#include <stddef.h>
#include <stdint.h>
#if defined(__GNUC__)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

#define CALLBACK_ARRAY_BYTES 64u
#define ENTRY_ARRAY_BYTES 32u
#define RECURSIVE_ARRAY_BYTES 16u
#define VLA_LIMIT_BYTES 256u

NOINLINE uint32_t ef_stack_callback(uint32_t value)
{
    volatile unsigned char bytes[CALLBACK_ARRAY_BYTES];
    for (unsigned i = 0; i < CALLBACK_ARRAY_BYTES; ++i)
    {
        bytes[i] = (unsigned char)(value + i);
    }
    return bytes[value % CALLBACK_ARRAY_BYTES];
}

NOINLINE uint32_t ef_stack_entry(uint32_t value)
{
    volatile unsigned char bytes[ENTRY_ARRAY_BYTES];
    for (unsigned i = 0; i < ENTRY_ARRAY_BYTES; ++i)
    {
        bytes[i] = (unsigned char)(value + i);
    }
    return ef_stack_callback(bytes[value % ENTRY_ARRAY_BYTES]) + bytes[0];
}

NOINLINE uint32_t ef_stack_recursive(unsigned depth)
{
    volatile unsigned char bytes[RECURSIVE_ARRAY_BYTES];
    bytes[0] = (unsigned char)depth;
    if (!depth)
    {
        return bytes[0];
    }
    return ef_stack_recursive(depth - 1) + bytes[0];
}

/* Returns 0 for a length of 0 or above VLA_LIMIT_BYTES, bounding the frame. */
NOINLINE uint32_t ef_stack_vla(size_t length)
{
    if (length == 0 || length > VLA_LIMIT_BYTES)
    {
        return 0;
    }
    volatile unsigned char bytes[length];
    for (size_t i = 0; i < length; ++i)
    {
        bytes[i] = (unsigned char)i;
    }
    return bytes[length - 1];
}

uint32_t ef_stack_tail(uint32_t value)
{
    return ef_stack_callback(value);
}

uint32_t ef_stack_indirect(uint32_t (*callback)(uint32_t), uint32_t value)
{
    return callback(value);
}

int main(void)
{
    int all_correct = ef_stack_entry(2) == 10 && ef_stack_recursive(3) == 6 &&
                      ef_stack_vla(16) == 15 && ef_stack_tail(2) == 4 &&
                      ef_stack_indirect(ef_stack_callback, 2) == 4;
    return all_correct ? 0 : 1;
}
