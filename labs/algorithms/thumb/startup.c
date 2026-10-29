#include "algo.h"
#ifndef ALGO_THUMB_CASE
#define ALGO_THUMB_CASE 0
#endif
extern uint32_t __stack_top, __data_start, __data_end, __data_load, __bss_start,
    __bss_end;
int algo_abi_insert_probe(const uint32_t *, uint32_t, uint32_t, uint32_t *);
int algo_abi_sift_probe(const uint32_t *, uint32_t, uint32_t, uint32_t *);
static uint32_t random_state = 0x5142387u;

static uint32_t random_word(void)
{
    random_state ^= random_state << 13;
    random_state ^= random_state >> 17;
    random_state ^= random_state << 5;
    return random_state;
}

static void semihost(uint32_t op, const void *arg)
{
    register uint32_t r0 __asm__("r0") = op;
    register const void *r1 __asm__("r1") = arg;
    __asm__ volatile("bkpt 0xab" : "+r"(r0) : "r"(r1) : "memory");
}

__attribute__((noreturn)) static void stop(int status)
{
    uint32_t exit_args[2] = {0x20026u, (uint32_t)status};
    semihost(4, status == 2 ? "ALGO_THUMB_FAULT\n"
                : status    ? "ALGO_THUMB_FAIL\n"
                            : "ALGO_THUMB_PASS\n");
    semihost(0x20, exit_args);
    for (;;)
    {
    }
}

#define REQUIRE(x)   \
    do               \
    {                \
        if (!(x))    \
            stop(1); \
    } while (0)

static void exercise(void)
{
    uint32_t out;
    REQUIRE(algo_abi_probe(0, 0, 3, &out) == 1 && out == 0);
    uint32_t bad[] = {2, 1};
    out = 99;
    REQUIRE(algo_checked_lower(bad, 2, 2, 1, &out) == ALGO_BAD && out == 99);
    REQUIRE(algo_checked_lower(bad, 2, 1, 1, &out) == ALGO_BAD && out == 99);
    REQUIRE(algo_checked_lower(0, 0, 0, 1, &out) == 0 && out == 0);
    if (ALGO_THUMB_CASE == 0 || ALGO_THUMB_CASE == 2)
    {
        algo_pair short_array[2] = {{2, 0}, {1, 1}};
        REQUIRE(algo_checked_insert(short_array, 2, 1) == ALGO_BAD &&
                short_array[0].key == 2);
        REQUIRE(algo_checked_insert(short_array, 2, 2) == 0 &&
                short_array[0].tag == 1);
    }
    if (ALGO_THUMB_CASE == 0 || ALGO_THUMB_CASE == 3)
    {
        uint32_t broken[] = {1, 2, 3, 9};
        REQUIRE(algo_checked_sift(broken, 4, 4, 0) == ALGO_BAD &&
                broken[0] == 1);
        uint32_t ancestor_bad[] = {10, 1, 9, 20, 0};
        const uint32_t ancestor_before[] = {10, 1, 9, 20, 0};
        REQUIRE(algo_checked_sift(ancestor_bad, 5, 5, 1) == ALGO_BAD);
        for (unsigned i = 0; i < 5; ++i)
            REQUIRE(ancestor_bad[i] == ancestor_before[i]);
        uint32_t internal[] = {30, 1, 9, 20, 0};
        const uint32_t internal_expected[] = {30, 20, 9, 1, 0};
        REQUIRE(algo_checked_sift(internal, 5, 5, 1) == ALGO_OK);
        for (unsigned i = 0; i < 5; ++i)
            REQUIRE(internal[i] == internal_expected[i]);
    }
    REQUIRE(algo_abi_probe(0, 0, 3, &out) == 1 && out == 0);
    for (unsigned trial = 0; trial < 500; ++trial)
    {
        algo_pair a[32], before[32];
        uint32_t words[32];
        unsigned n = random_word() % 33;
        for (unsigned i = 0; i < n; ++i)
        {
            a[i] = (algo_pair){
                (random_word() % 9) ^ (random_word() & 0x80000000u), i};
            before[i] = a[i];
        }
        if (ALGO_THUMB_CASE == 0 || ALGO_THUMB_CASE == 2)
        {
            REQUIRE(algo_abi_insert_probe((const uint32_t *)a, n, 0, &out) ==
                    1);
        }
        else
        {
            /* Prepare sorted search input independently of the future insertion lab.
             */
            for (unsigned i = 0; i < n; ++i)
            {
                unsigned least = i;
                for (unsigned j = i + 1; j < n; ++j)
                    if (a[j].key < a[least].key ||
                        (a[j].key == a[least].key && a[j].tag < a[least].tag))
                        least = j;
                algo_pair saved = a[i];
                a[i] = a[least];
                a[least] = saved;
            }
        }
        uint32_t seen = 0;
        for (unsigned i = 0; i < n; ++i)
        {
            REQUIRE(a[i].tag < n && !(seen & (1u << a[i].tag)));
            seen |= 1u << a[i].tag;
            REQUIRE(before[a[i].tag].key == a[i].key);
            if (i)
                REQUIRE(a[i - 1].key < a[i].key ||
                        (a[i - 1].key == a[i].key && a[i - 1].tag < a[i].tag));
            words[i] = a[i].key;
        }
        for (unsigned q = 0; q < 12; ++q)
        {
            uint32_t key = q == 0 ? UINT32_MAX : random_word(), expected = 0;
            while (expected < n && words[expected] < key)
                ++expected;
            REQUIRE(algo_abi_probe(words, n, key, &out) == 1 &&
                    out == expected);
        }
        if (ALGO_THUMB_CASE == 0 || ALGO_THUMB_CASE == 3)
        {
            uint32_t heap[33], original[33];
            for (unsigned i = 0; i < 33; ++i)
                heap[i] = 1000 - i;
            heap[0] = random_word() % 900;
            for (unsigned i = 0; i < 33; ++i)
                original[i] = heap[i];
            REQUIRE(algo_abi_sift_probe(heap, 33, 0, &out) == 1);
            for (unsigned i = 1; i < 33; ++i)
                REQUIRE(heap[(i - 1) / 2] >= heap[i]);
            /* Independent multiset count, deliberately quadratic and bounded. */
            for (unsigned i = 0; i < 33; ++i)
            {
                unsigned a_count = 0, b_count = 0;
                for (unsigned j = 0; j < 33; ++j)
                {
                    a_count += original[j] == original[i];
                    b_count += heap[j] == original[i];
                }
                REQUIRE(a_count == b_count);
            }
        }
    }
    static uint32_t repeated[1024];
    for (unsigned i = 0; i < 1024; ++i)
        repeated[i] = 0x80000000u;
    REQUIRE(algo_abi_probe(repeated, 1024, UINT32_MAX, &out) == 1 &&
            out == 1024);
    REQUIRE(algo_asm_lower(repeated, 1024, 0x80000000u) == 0);
}

void reset(void)
{
    /* Fixed Cortex-M4/link.ld profile: numeric linker addresses describe aligned
     * half-open intervals. Their integer-to-pointer accesses are target-defined;
     * separately declared linker symbols are not one ISO C array object. */
    uintptr_t src = (uintptr_t)&__data_load;
    for (uintptr_t p = (uintptr_t)&__data_start; p < (uintptr_t)&__data_end;
         p += sizeof(uint32_t), src += sizeof(uint32_t))
        *(uint32_t *)p = *(const uint32_t *)src;
    for (uintptr_t p = (uintptr_t)&__bss_start; p < (uintptr_t)&__bss_end;
         p += sizeof(uint32_t))
        *(uint32_t *)p = 0;
    exercise();
    stop(0);
}

static void fault(void)
{
    stop(2);
}

__attribute__((section(".vectors"), used)) const void *vectors[] = {
    &__stack_top, reset, fault, fault, fault, fault, fault, 0, 0, 0, 0,
    fault,        fault, 0,     fault, fault};
