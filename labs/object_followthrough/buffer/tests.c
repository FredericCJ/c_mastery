#include "buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static void *owned[16];

static void cleanup(void)
{
    for (size_t i = 0; i < 16; ++i)
    {
        free(owned[i]);
        owned[i] = NULL;
    }
}

#define REQUIRE(x)                                 \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "OB_ASSERT:%s\n", #x); \
            cleanup();                             \
            return 41;                             \
        }                                          \
    } while (0)

struct ob_trace
{
    unsigned calls, releases, fail_call, move, resize_calls, same_bits;
};

static void *allocate(void *context, size_t bytes)
{
    struct ob_trace *t = context;
    void *p;
    if (++t->calls == t->fail_call)
    {
        return NULL;
    }
    p = malloc(bytes);
    if (p)
    {
        for (size_t i = 0; i < 16; ++i)
        {
            if (!owned[i])
            {
                owned[i] = p;
                return p;
            }
        }
        free(p);
    }
    return NULL;
}

static void release(void *context, void *p)
{
    struct ob_trace *t = context;
    ++t->releases;
    for (size_t i = 0; i < 16; ++i)
    {
        if (owned[i] == p)
        {
            free(p);
            owned[i] = NULL;
            return;
        }
    }
}

static void *resize(void *context, void *old, size_t oldbytes, size_t newbytes)
{
    struct ob_trace *t = context;
    size_t slot = 16;
    void *p;
    for (size_t i = 0; i < 16; ++i)
    {
        if (owned[i] == old)
        {
            slot = i;
            break;
        }
    }
    if (slot == 16)
    {
        return NULL;
    }
    ++t->resize_calls;
    if (++t->calls == t->fail_call)
    {
        return NULL;
    }
    if (t->move)
    {
        p = malloc(newbytes);
        if (!p)
        {
            return NULL;
        }
        for (size_t i = 0; i < oldbytes && i < newbytes; ++i)
        {
            ((unsigned char *)p)[i] = ((unsigned char *)old)[i];
        }
        free(old);
    }
    else
    {
#ifdef UINTPTR_MAX
        uintptr_t bits = (uintptr_t)old;
#endif
        p = realloc(old, newbytes);
        if (!p)
        {
            return NULL;
        }
#ifdef UINTPTR_MAX
        t->same_bits = (uintptr_t)p == bits;
#endif
    }
    owned[slot] = p;
    return p;
}

static struct ob_allocator allocator(struct ob_trace *t)
{
    return (struct ob_allocator){t, allocate, resize, release};
}

static int initialization(void)
{
    struct ob_trace t = {0};
    struct ob_allocator a = allocator(&t);
    struct ob_construction c;
    unsigned char wire[8];
    struct ob_record other = {258, NULL, 'A'};
    REQUIRE(ob_construct(&c, 258, NULL, 'A', 0) == 0 && c.initialized == 7 &&
            ob_record_equal(&c.value, &other));
    memset(wire, 0xa5, sizeof wire);
    REQUIRE(ob_record_encode(&c.value, wire, sizeof wire) == 0 &&
            wire[0] == 1 && wire[1] == 2 && wire[2] == 'A' && wire[3] == 0);
    for (size_t i = 4; i < 8; ++i)
    {
        REQUIRE(wire[i] == 0xa5);
    }
    REQUIRE(ob_cleanup(&c, a) == 0 && !c.initialized && !t.releases);
    REQUIRE(ob_construct(&c, 17, NULL, 'B', 1) == -1 && c.initialized == 1);
    REQUIRE(ob_cleanup(&c, a) == 0 && !t.releases);
    {
        void *child = allocate(&t, 4);
        REQUIRE(child != NULL);
        REQUIRE(ob_construct(&c, 17, child, 'B', 2) == -1 &&
                c.initialized == 3);
        REQUIRE(ob_cleanup(&c, a) == 0 && !c.initialized && t.releases == 1);
    }
    puts("OB_INITIALIZATION_PASS");
    cleanup();
    return 0;
}

static int sizing(void)
{
    struct ob_trace t = {0};
    struct ob_table table = {0};
    size_t out = 99;
    REQUIRE(ob_size(16, 4, 0, 255, &out) == 0 && out == 16);
    REQUIRE(ob_size(16, 4, 3, 255, &out) == 0 && out == 28);
    REQUIRE(ob_size(16, 4, 100, 255, &out) == -1 && out == 28);
    REQUIRE(ob_size(0, 1, SIZE_MAX, SIZE_MAX, &out) == 0 && out == SIZE_MAX);
    REQUIRE(ob_size(1, 1, SIZE_MAX, SIZE_MAX, &out) == -1 && out == SIZE_MAX);
    REQUIRE(ob_size(0, 0, 1, 255, &out) == -1 && out == SIZE_MAX);
    REQUIRE(ob_create(&table, allocator(&t), 11, 0) == 0 &&
            table.value->length == 0 && table.value->capacity == 0);
    REQUIRE(ob_append(&table, 1) == -1);
    ob_reset(&table);
    REQUIRE(t.calls == 1 && t.releases == 1);
    printf(
        "OB_LAYOUT:flex_size=%zu,flex_offset=%zu,padded_size=%zu,padded_offset=%zu,padded_alignment=%zu\n",
        sizeof(struct ob_flex), offsetof(struct ob_flex, payload),
        sizeof(struct ob_padded), offsetof(struct ob_padded, payload),
        _Alignof(struct ob_padded));
    REQUIRE(sizeof(struct ob_padded) > offsetof(struct ob_padded, payload));
    {
        size_t size;
        struct ob_padded *p;
        REQUIRE(ob_size(sizeof *p, 1, 3, SIZE_MAX, &size) == 0);
        p = allocate(&t, size);
        REQUIRE(p != NULL);
        p->alignment = 1.0;
        p->tag = 3;
        for (size_t i = 0; i < 3; ++i)
        {
            p->payload[i] = (unsigned char)(i + 1);
        }
        REQUIRE(p->payload[2] == 3);
        release(&t, p);
    }
    cleanup();
    puts("OB_SIZING_PASS");
    return 0;
}

static int workspace(void)
{
    int storage[16], sum = 77;
    unsigned evaluations = 99;
    REQUIRE(ob_workspace(0, storage, 16, &sum, &evaluations) == -1 &&
            sum == 77 && evaluations == 99);
    REQUIRE(ob_workspace(-1, storage, 16, &sum, &evaluations) == -1);
    REQUIRE(ob_workspace(17, storage, 16, &sum, &evaluations) == -1);
    REQUIRE(ob_workspace(4, storage, 3, &sum, &evaluations) == -1);
    for (int n = 1; n <= 16; ++n)
    {
        REQUIRE(ob_workspace(n, storage, 16, &sum, &evaluations) == 0 &&
                sum == n * (n + 1) / 2);
#if !defined(OB_NO_VLA) && !defined(__STDC_NO_VLA__)
        REQUIRE(evaluations == 1);
#else
        REQUIRE(evaluations == 0);
#endif
    }
    printf("OB_VLA:standard=%ld,evaluations=%u\n", (long)__STDC_VERSION__,
           evaluations);
    puts("OB_WORKSPACE_PASS");
    return 0;
}

static int representations(void)
{
    struct ob_trace t = {0};
    struct ob_allocator a = allocator(&t);
    struct ob_table flex = {0};
    struct ob_split *split = NULL;
    struct ob_shared *first = NULL, *second = NULL, *third = NULL,
                     *fourth = NULL;
    unsigned char source[3] = {3, 5, 7};
    t.fail_call = 1;
    REQUIRE(ob_create(&flex, a, 11, 3) == -1 && !flex.value);
    t.fail_call = 0;
    REQUIRE(ob_create(&flex, a, 11, 3) == 0 && t.calls == 2);
    for (size_t i = 0; i < 3; ++i)
    {
        REQUIRE(ob_append(&flex, source[i]) == 0);
    }
    t.fail_call = t.calls + 2;
    REQUIRE(ob_split_create(a, 3, &split) == -1 && !split && t.releases == 1);
    t.fail_call = 0;
    REQUIRE(ob_split_create(a, 3, &split) == 0);
    for (size_t i = 0; i < 3; ++i)
    {
        split->payload[i] = source[i];
        ++split->length;
        REQUIRE(split->payload[i] == flex.value->payload[i]);
    }
    REQUIRE(ob_shared_create(a, source, 3, &first) == 0);
    REQUIRE(ob_shared_retain(first, &second) == 0 && first->references == 2);
    REQUIRE(ob_shared_retain(first, &third) == 0 && first->references == 3);
    REQUIRE(ob_shared_retain(first, &fourth) == -1 && !fourth &&
            first->references == 3);
    ob_shared_drop(&first);
    REQUIRE(!first && second->references == 2 && second->payload[2] == 7);
    ob_shared_drop(&second);
    REQUIRE(!second && third->references == 1);
    ob_shared_drop(&third);
    REQUIRE(!third);
    ob_split_drop(&split);
    ob_reset(&flex);
    REQUIRE(!split && !flex.value && t.releases == 5);
    printf(
        "OB_STORAGE:flex_allocations=1,split_allocations=2,shared_payload_copies=1,maximum_references=3\n");
    cleanup();
    puts("OB_REPRESENTATIONS_PASS");
    return 0;
}

static int relocation(void)
{
    struct ob_trace t = {0};
    struct ob_table table = {0};
    struct ob_handle h = {0};
    unsigned char out = 99;
    struct ob_allocator a = allocator(&t);
    size_t calls;
    REQUIRE(ob_create(&table, a, 11, 4) == 0 && ob_append(&table, 3) == 0 &&
            ob_append(&table, 5) == 0 && ob_handle(&table, 0, &h) == 0);
    {
        unsigned char *view = table.value->payload;
        REQUIRE(ob_append(&table, 7) == 0 && view[0] == 3 &&
                ob_resolve(&table, h, &out) == 0 && out == 3);
    }
    calls = t.calls;
    REQUIRE(ob_resize(&table, 0) == -1 && t.calls == calls &&
            table.generation == 1);
    t.fail_call = t.calls + 1;
    {
        struct ob_flex *owner = table.value;
        REQUIRE(ob_resize(&table, 8) == -1 && table.value == owner &&
                table.generation == 1 && table.value->capacity == 4 &&
                table.value->length == 3);
        REQUIRE(ob_resolve(&table, h, &out) == 0 && out == 3);
    }
    t.fail_call = 0;
    t.move = 1;
    REQUIRE(ob_resize(&table, 8) == 0 && table.generation == 2 &&
            table.value->capacity == 8 && table.value->length == 3 &&
            table.value->payload[2] == 7);
    REQUIRE(ob_resolve(&table, h, &out) == -1 && out == 3);
    REQUIRE(ob_handle(&table, 1, &h) == 0);
    t.move = 0;
    REQUIRE(ob_resize(&table, 8) == 0 && table.generation == 3);
    REQUIRE(ob_resolve(&table, h, &out) == -1 && out == 3);
    printf("OB_REALLOC:numerical_address_reused=%u\n", t.same_bits);
    calls = t.calls;
    REQUIRE(ob_resize(&table, 0) == -1 && ob_resize(&table, 9) == -1 &&
            t.calls == calls);
    ob_reset(&table);
    REQUIRE(!table.value && table.retired && ob_create(&table, a, 11, 4) == -1);
    cleanup();
    puts("OB_RELOCATION_PASS");
    return 0;
}

static int handles(void)
{
    struct ob_trace t = {0};
    struct ob_table a = {0}, b = {0};
    struct ob_handle first = {0}, second = {0};
    unsigned char out = 99;
    REQUIRE(ob_create(&a, allocator(&t), 17, 4) == 0);
    for (unsigned char i = 1; i <= 3; ++i)
    {
        REQUIRE(ob_append(&a, i) == 0);
    }
    REQUIRE(ob_handle(&a, 1, &first) == 0);
    {
        unsigned char *same_slot = a.value->payload + 1;
        REQUIRE(ob_erase(&a, 0) == 0 && a.generation == 2 && *same_slot == 3);
    }
    REQUIRE(ob_resolve(&a, first, &out) == -1 && out == 99);
    REQUIRE(ob_handle(&a, 1, &second) == 0 &&
            ob_resolve(&a, second, &out) == 0 && out == 3);
    REQUIRE(ob_table_move(&b, &a) == 0 && !a.value && a.retired &&
            b.identity == 17 && b.generation == 2);
    REQUIRE(ob_resolve(&a, second, &out) == -1);
    REQUIRE(ob_resolve(&b, second, &out) == 0 && out == 3);
    REQUIRE(ob_erase(&b, 0) == 0 && b.generation == 3 && b.value->length == 1);
    REQUIRE(ob_erase(&b, 0) == -1 && b.value->length == 1);
    ob_reset(&b);
    REQUIRE(ob_resolve(&b, second, &out) == -1 && t.releases == 1);
    cleanup();
    puts("OB_HANDLES_PASS");
    return 0;
}

int main(int argc, char **argv)
{
    switch (argc == 2 ? atoi(argv[1]) : -1)
    {
        case 0:
            return initialization();
        case 1:
            return sizing();
        case 2:
            return workspace();
        case 3:
            return representations();
        case 4:
            return relocation();
        case 5:
            return handles();
        default:
            return 2;
    }
}
