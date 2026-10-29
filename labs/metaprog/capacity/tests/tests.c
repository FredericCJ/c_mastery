#include "capacity.h"
#include "metaprog.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <time.h>
#define CHECK(x)                                                             \
    do                                                                       \
    {                                                                        \
        if (!(x))                                                            \
        {                                                                    \
            fprintf(stderr, "MPO_CHECK %s:%d %s\n", __FILE__, __LINE__, #x); \
            return 1;                                                        \
        }                                                                    \
    } while (0)

static int fixed(void)
{
    struct mp_mpc1 one = {0};
    struct mp_mpc4 four = {0};
    struct mp_mpc8 eight = {0};
    uint32_t out = 99;
    for (unsigned turn = 0; turn < 16; turn++)
    {
        CHECK(mpc1_push(&one, turn) == MPC_OK &&
              mpc1_push(&one, 99) == MPC_FULL &&
              mpc1_pop(&one, &out) == MPC_OK && out == turn);
        for (unsigned j = 0; j < 4; j++)
            CHECK(mpc4_push(&four, turn * 4 + j) == MPC_OK);
        CHECK(mpc4_push(&four, 99) == MPC_FULL &&
              mpc4_pop(&four, &out) == MPC_OK && out == turn * 4);
        CHECK(mpc4_push(&four, turn * 4 + 4) == MPC_OK);
        for (unsigned j = 1; j <= 4; j++)
            CHECK(mpc4_pop(&four, &out) == MPC_OK && out == turn * 4 + j);
        for (unsigned j = 0; j < 8; j++)
            CHECK(mpc8_push(&eight, j) == MPC_OK);
        for (unsigned j = 0; j < 8; j++)
            CHECK(mpc8_pop(&eight, &out) == MPC_OK && out == j);
    }
    out = 99;
    CHECK(mpc4_pop(&four, &out) == MPC_EMPTY && out == 99);
    puts("MPC_FIXED capacities1-4-8 masked-wrap FIFO");
    return 0;
}

static int runtime(void)
{
    for (size_t cap = 1; cap <= 17; cap++)
    {
        uint32_t storage[17];
        struct mpc_runtime r;
        volatile size_t supplied = cap;
        CHECK(mpc_init(&r, storage, supplied, sizeof storage) == MPC_OK &&
              r.power2 == ((cap & (cap - 1)) == 0));
        for (unsigned turn = 0; turn < 3; turn++)
        {
            uint32_t out = 99;
            CHECK(mpc_pop(&r, &out) == MPC_EMPTY && out == 99);
            for (size_t j = 0; j < cap; j++)
                CHECK(mpc_push(&r, (uint32_t)j) == MPC_OK);
            CHECK(mpc_push(&r, 99) == MPC_FULL);
            CHECK(mpc_pop(&r, &out) == MPC_OK && out == 0 &&
                  mpc_push(&r, 77) == MPC_OK);
            for (size_t j = 1; j < cap; j++)
                CHECK(mpc_pop(&r, &out) == MPC_OK && out == j);
            CHECK(mpc_pop(&r, &out) == MPC_OK && out == 77);
        }
    }
    puts("MPC_RUNTIME volatile-capacity arbitrary-positive fallback");
    return 0;
}

static int refusal(void)
{
    uint32_t storage[4] = {1, 2, 3, 4};
    struct mpc_runtime r = {storage, 99, 3, 2, 98, 0}, before = r;
    CHECK(mpc_init(&r, storage, 4, 15) == MPC_RANGE &&
          r.capacity == before.capacity && r.head == before.head &&
          r.count == before.count && r.data == before.data);
    CHECK(mpc_init(&r, storage, 0, 16) == MPC_RANGE && r.capacity == 99);
    CHECK(mpc_init(&r, storage, SIZE_MAX, 16) == MPC_RANGE && r.capacity == 99);
    CHECK(mpc_init(&r, NULL, 4, 16) == MPC_INVALID && r.capacity == 99);
    CHECK(mpc_init(&r, storage, 4, 16) == MPC_OK && r.capacity == 4 &&
          r.count == 0 && storage[0] == 1 && storage[3] == 4);
    puts("MPC_REFUSAL unchanged-control byte-budget zero-overflow");
    return 0;
}

static int baseline(void)
{
    size_t out = 99;
    CHECK(!mp_storage_bound(SIZE_MAX, 2, SIZE_MAX, &out) && out == 99);
    CHECK(!mp_storage_bound(4, 8, 31, &out) && out == 99);
    CHECK(mp_storage_bound(4, 8, 32, &out) && out == 32);
    CHECK(!mp_storage_bound(0, 1, 32, &out) && out == 32);
    puts("MPC_BASE actual-checked-product independent-budget");
    return 0;
}

static int histories(void)
{
    size_t cap;
    char word[65];
    while (scanf("%zu %64s", &cap, word) == 2)
    {
        CHECK(cap > 0 && cap <= 17);
        uint32_t data[17];
        struct mpc_runtime r;
        CHECK(mpc_init(&r, data, cap, sizeof data) == MPC_OK);
        printf("[");
        for (size_t i = 0; word[i]; i++)
        {
            uint32_t out = 99;
            int status = word[i] == 'P' ? mpc_pop(&r, &out)
                                        : mpc_push(&r, word[i] == 'B' ? 2 : 1);
            printf("%s[%d,%zu,%zu,%u]", i ? "," : "", status, r.head, r.count,
                   out);
        }
        puts("]");
    }
    return ferror(stdin) ? 1 : 0;
}

static int arithmetic(void)
{
    uint64_t count, width, limit;
    while (scanf("%" SCNu64 " %" SCNu64 " %" SCNu64, &count, &width, &limit) ==
           3)
    {
        size_t out = 99;
        int ok =
            mp_storage_bound((size_t)count, (size_t)width, (size_t)limit, &out);
        printf("%d %zu\n", ok, out);
    }
    return ferror(stdin) ? 1 : 0;
}

static int metrics(void)
{
    double seconds[2][7];
    uint64_t sum = 0;
    for (unsigned mode = 0; mode < 2; mode++)
        for (unsigned sample = 0; sample < 7; sample++)
        {
            clock_t start = clock();
            struct mp_mpc4 fixed = {0};
            struct mpc_runtime r;
            uint32_t storage[4];
            CHECK(mpc_init(&r, storage, 4, sizeof storage) == MPC_OK);
            sum = 0;
            for (unsigned i = 0; i < 100000; i++)
            {
                for (unsigned j = 0; j < 4; j++)
                    CHECK((mode ? mpc_push(&r, j) : mpc4_push(&fixed, j)) ==
                          MPC_OK);
                for (unsigned j = 0; j < 4; j++)
                {
                    uint32_t out;
                    CHECK((mode ? mpc_pop(&r, &out) : mpc4_pop(&fixed, &out)) ==
                          MPC_OK);
                    sum += out;
                }
            }
            seconds[mode][sample] = (double)(clock() - start) / CLOCKS_PER_SEC;
            CHECK(sum == 600000);
        }
    printf(
        "{\"iterations\":100000,\"checksum\":%" PRIu64
        ",\"size_t_bits\":%zu,\"fixed_size\":%zu,\"runtime_control_size\":%zu,\"runtime_backing_size\":%zu",
        sum, sizeof(size_t) * 8, sizeof(struct mp_mpc4),
        sizeof(struct mpc_runtime), sizeof(uint32_t) * 4);
    for (unsigned m = 0; m < 2; m++)
    {
        printf(",\"%s\":[", m ? "runtime_seconds" : "fixed_seconds");
        for (unsigned i = 0; i < 7; i++)
            printf("%s%.9f", i ? "," : "", seconds[m][i]);
        printf("]");
    }
    puts("}");
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "capacity_fixed"))
        return fixed();
    if (!strcmp(argv[1], "capacity_runtime"))
        return runtime();
    if (!strcmp(argv[1], "capacity_refusal"))
        return refusal();
    if (!strcmp(argv[1], "capacity_base"))
        return baseline();
    if (!strcmp(argv[1], "histories"))
        return histories();
    if (!strcmp(argv[1], "arithmetic"))
        return arithmetic();
    if (!strcmp(argv[1], "metrics"))
        return metrics();
    return 2;
}
