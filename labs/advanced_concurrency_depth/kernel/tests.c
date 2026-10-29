#include "kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                   \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            fprintf(stderr, "AK_ASSERT:%s\n", #x); \
            exit(41);                              \
        }                                          \
    } while (0)

struct ag_copy
{
    size_t residual, calls;
};

static size_t copying(void *p, unsigned char *out, size_t n)
{
    struct ag_copy *c = p;
    size_t i;
    ++c->calls;
    for (i = 0; i < n - c->residual; ++i)
    {
        out[i] = (unsigned char)(i + 1);
    }
    return c->residual;
}

static void copy_test(void)
{
    struct ak_packet p;
    struct ag_copy c;
    size_t n, r, i;
    memset(&p, 0x5a, sizeof p);
    p.length = 17;
    CHECK(ak_copy(&p, 0, copying, &c) == AK_INVALID);
    CHECK(ak_copy(&p, 33, copying, &c) == AK_INVALID);
    for (n = 1; n <= 32; ++n)
    {
        for (r = 0; r <= n; ++r)
        {
            struct ak_packet old = p;
            c = (struct ag_copy){r, 0};
            CHECK(ak_copy(&p, n, copying, &c) == (r ? AK_FAULT : AK_OK));
            CHECK(c.calls == 1);
            if (r)
            {
                CHECK(memcmp(&old, &p, sizeof p) == 0);
            }
            else
            {
                CHECK(p.length == n);
                for (i = 0; i < n; ++i)
                {
                    CHECK(p.bytes[i] == i + 1);
                }
                for (; i < 32; ++i)
                {
                    CHECK(p.bytes[i] == 0);
                }
            }
        }
    }
    puts("OBS copy partial_boundaries=560 zero_padding_not_success=1");
}

struct ag_device
{
    unsigned mapped, device, ready, stage, fail_map, fail_submit, maps, unmaps,
        errors;
    struct as_value device_value;
};

static int map(void *p, const struct as_value *v, unsigned *c)
{
    struct ag_device *d = p;
    if (d->fail_map)
    {
        return -1;
    }
    CHECK(!d->mapped);
    d->mapped = 1;
    d->stage = 1;
    d->device_value = *v;
    *c = ++d->maps;
    return 0;
}

static void sync_device(void *p, unsigned c)
{
    struct ag_device *d = p;
    CHECK(c == d->maps && d->mapped && d->stage == 1);
    d->stage = 2;
}

static int submit(void *p, unsigned c)
{
    struct ag_device *d = p;
    CHECK(c == d->maps && d->stage == 2);
    if (d->fail_submit)
    {
        return -1;
    }
    d->device = 1;
    d->stage = 3;
    return 0;
}

static int quiesce(void *p, unsigned c)
{
    struct ag_device *d = p;
    CHECK(c == d->maps && d->device);
    if (!d->ready)
    {
        return 0;
    }
    d->device = 0;
    d->stage = 4;
    return 1;
}

static void sync_cpu(void *p, unsigned c)
{
    struct ag_device *d = p;
    CHECK(c == d->maps && !d->device && d->stage == 4);
    d->stage = 5;
}

static void unmap(void *p, unsigned c)
{
    struct ag_device *d = p;
    CHECK(c == d->maps && d->mapped && !d->device &&
          (d->stage == 2 || d->stage == 5));
    d->mapped = 0;
    d->unmaps++;
    d->stage = 0;
}

static void backend_test(void)
{
    struct ak_service s;
    struct ag_device d = {0};
    struct ak_backend b = {map,      sync_device, submit, quiesce,
                           sync_cpu, unmap,       &d};
    struct as_value out = {99, 99, 99, 99}, v = {1, ~1u, 3, 3};
    unsigned i;
    CHECK(ak_init(&s, b) == AK_OK);
    CHECK(ak_start(&s) == AK_EMPTY);
    for (i = 0; i < 4; ++i)
    {
        v.sequence = i;
        CHECK(ak_enqueue(&s, v) == AK_OK);
    }
    CHECK(ak_enqueue(&s, v) == AK_BUSY);
    d.fail_map = 1;
    CHECK(ak_start(&s) == AK_IO && s.state == 1 && !d.mapped);
    d.fail_map = 0;
    d.fail_submit = 1;
    CHECK(ak_start(&s) == AK_IO && s.state == 1 && d.unmaps == 1);
    d.fail_submit = 0;
    for (i = 0; i < 4; ++i)
    {
        CHECK(ak_start(&s) == AK_OK);
        CHECK(d.device_value.sequence == i);
        CHECK(ak_start(&s) == AK_BUSY);
        d.ready = 0;
        CHECK(ak_complete(&s, &out) == AK_BUSY);
        CHECK(out.sequence == (i ? i - 1 : 99) && d.mapped && d.device);
        d.ready = 1;
        CHECK(ak_complete(&s, &out) == AK_OK && out.sequence == i &&
              !d.mapped && !d.device);
        CHECK(ak_complete(&s, &out) == AK_BUSY);
    }
    CHECK(ak_start(&s) == AK_EMPTY && d.maps == 5 && d.unmaps == 5);
    puts("OBS backend actual_spsc=1 map_retry=1 submit_rollback=1 "
         "quiescence_before_reuse=1 physical=0");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "copy"))
    {
        copy_test();
    }
    else if (!strcmp(argv[1], "backend"))
    {
        backend_test();
    }
    else
    {
        CHECK(0);
    }
    return 0;
}
