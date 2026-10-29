#include "allocation.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "ASSERT heap line %d: %s\n", __LINE__, #x); \
            return 1;                                                   \
        }                                                               \
    } while (0)

static unsigned next(unsigned *s)
{
    *s = *s * 1664525u + 1013904223u;
    return *s;
}

int main(void)
{
    struct al_heap h, copy;
    struct al_handle a, b, c, out = {0};
    unsigned char *p;
    size_t n, total, big;
    CHECK(al_heap_init(&h, 64, 0, 0, 1) == AL_OK);
    CHECK(al_heap_take(&h, 4, 8, &a) == AL_OK);
    CHECK(al_heap_view(&h, a, &p, &n) == AL_OK && n == 4);
    p[0] = 10;
    p[1] = 20;
    p[2] = 30;
    p[3] = 40;
    CHECK(al_heap_take(&h, 4, 8, &b) == AL_OK);
    copy = h;
    CHECK(al_heap_resize(&h, a, 20, 1, SIZE_MAX, &out) == AL_FULL &&
          memcmp(&h, &copy, sizeof h) == 0);
    CHECK(al_heap_resize(&h, a, 20, 0, 2, &out) == AL_CLEANUP &&
          memcmp(&h, &copy, sizeof h) == 0);
    CHECK(al_heap_resize(&h, a, 20, 0, SIZE_MAX, &c) == AL_OK);
    CHECK(al_heap_view(&h, a, &p, &n) == AL_STALE);
    CHECK(al_heap_view(&h, c, &p, &n) == AL_OK && n == 20 && p[0] == 10 &&
          p[3] == 40);
    CHECK(al_heap_release(&h, b) == AL_OK);
    CHECK(al_heap_release(&h, c) == AL_OK);
    CHECK(al_heap_stats(&h, &total, &big) == AL_OK && total == 64 && big == 64);
    CHECK(al_heap_take(&h, 4, 1, &a) == AL_OK);
    CHECK(al_heap_resize(&h, a, 25, 0, SIZE_MAX, &b) == AL_OK);
    CHECK(al_heap_resize(&h, b, 1, 0, SIZE_MAX, &c) == AL_OK);
    CHECK(al_heap_check(&h) == AL_OK);
    CHECK(al_heap_release(&h, c) == AL_OK);
    for (unsigned size = 1; size <= 52; ++size)
    {
        CHECK(al_heap_init(&h, 64, 0, 0, 1) == AL_OK);
        CHECK(al_heap_take(&h, size, 8, &a) == AL_OK);
        CHECK(al_heap_check(&h) == AL_OK);
        CHECK(al_heap_release(&h, a) == AL_OK && h.free_count == 1);
    }
    CHECK(al_heap_init(&h, 64, 0, 1, 1) == AL_OK);
    CHECK(al_heap_take(&h, 4, 8, &a) == AL_OK);
    CHECK(al_heap_take(&h, 4, 8, &b) == AL_OK);
    CHECK(al_heap_release(&h, a) == AL_OK);
    CHECK(al_heap_release(&h, b) == AL_OK && h.free_count == 3);
    CHECK(al_heap_collect(&h) == AL_OK && h.free_count == 1);
    /* The checker must parse the physical partition, not trust the free index. */
    copy = h;
    h.bytes[60] = 8;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    h = copy;
    h.free_offsets[1] = 0;
    h.free_count = 2;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    h = copy;
    h.bytes[0] = 255;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    h = copy;
    h.capacity = 15;
    CHECK(al_heap_check(&h) == AL_CORRUPT);
    unsigned operations = 0;
    for (unsigned policy = 0; policy < 2; ++policy)
        for (unsigned seed = 1; seed <= 96; ++seed)
        {
            CHECK(al_heap_init(&h, 256, policy, 0, 1) == AL_OK);
            struct al_handle live[12] = {{0}};
            unsigned lengths[12] = {0};
            unsigned state = seed;
            for (unsigned step = 0; step < 256; ++step)
            {
                unsigned index = next(&state) % 12, op = next(&state) % 3;
                unsigned request = 1 + next(&state) % 49;
                copy = h;
                if (!lengths[index])
                {
                    int result = al_heap_take(&h, request,
                                              1u << (next(&state) % 4), &out);
                    CHECK(result == AL_OK || result == AL_FULL);
                    if (result == AL_OK)
                    {
                        live[index] = out;
                        lengths[index] = request;
                        CHECK(al_heap_view(&h, out, &p, &n) == AL_OK);
                        for (unsigned k = 0; k < n; ++k)
                            p[k] = (unsigned char)(index * 17 + k);
                    }
                    else
                        CHECK(memcmp(&h, &copy, sizeof h) == 0);
                }
                else if (op == 0)
                {
                    CHECK(al_heap_release(&h, live[index]) == AL_OK);
                    CHECK(al_heap_release(&h, live[index]) == AL_STALE);
                    lengths[index] = 0;
                }
                else
                {
                    int result =
                        al_heap_resize(&h, live[index], request, op == 1,
                                       op == 1 ? 0 : SIZE_MAX, &out);
                    CHECK(result == AL_OK || result == AL_FULL ||
                          result == AL_CLEANUP);
                    if (result == AL_OK)
                    {
                        CHECK(al_heap_view(&h, live[index], &p, &n) ==
                              AL_STALE);
                        CHECK(al_heap_view(&h, out, &p, &n) == AL_OK);
                        unsigned retained =
                            lengths[index] < request ? lengths[index] : request;
                        for (unsigned k = 0; k < retained; ++k)
                            CHECK(p[k] == (unsigned char)(index * 17 + k));
                        for (unsigned k = retained; k < n; ++k)
                            p[k] = (unsigned char)(index * 17 + k);
                        live[index] = out;
                        lengths[index] = request;
                    }
                    else
                        CHECK(memcmp(&h, &copy, sizeof h) == 0);
                }
                CHECK(al_heap_check(&h) == AL_OK);
                unsigned occupancy[256] = {0};
                size_t payload = 0;
                for (unsigned j = 0; j < 12; ++j)
                    if (lengths[j])
                    {
                        CHECK(al_heap_view(&h, live[j], &p, &n) == AL_OK &&
                              n == lengths[j]);
                        size_t off = (size_t)(p - h.bytes);
                        CHECK(off % 8 == 0 && off + n <= h.capacity);
                        for (unsigned k = 0; k < n; ++k)
                        {
                            CHECK(!occupancy[off + k]);
                            occupancy[off + k] = 1;
                            CHECK(p[k] == (unsigned char)(j * 17 + k));
                        }
                        payload += n;
                    }
                CHECK(h.payload == payload);
                ++operations;
            }
            for (unsigned j = 0; j < 12; ++j)
                if (lengths[j])
                    CHECK(al_heap_release(&h, live[j]) == AL_OK);
            CHECK(al_heap_stats(&h, &total, &big) == AL_OK && total == 256 &&
                  big == 256);
        }
    printf("PASS heap operations=%u patterns/nonoverlap/failure/metadata\n",
           operations);
    return 0;
}
