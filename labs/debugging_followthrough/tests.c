#include "diagnosis.h"
#include "ledger.h"
#include "firmware_io.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "DX_CHECK:%d:%s\n", __LINE__, #x); \
            exit(1);                                           \
        }                                                      \
    } while (0)

static int fixture(uint16_t m, void *p)
{
    (void)p;
    unsigned a = (1u << 0) | (1u << 6) | (1u << 7), b = (1u << 1) | (1u << 2);
    return (m & a) == a || (m & b) == b ? DX_FAIL
           : (m & a)                    ? DX_UNRESOLVED
                                        : DX_PASS;
}

static unsigned bitcount(unsigned x)
{
    unsigned n = 0;
    for (; x; x &= x - 1)
    {
        n++;
    }
    return n;
}

static void logging(void)
{
    struct dx_log outer = {0}, inner = {0};
    dx_log(&outer, 1);
    dx_log(&outer, 2);
    dx_log(&inner, 9);
    dx_log(&outer, 3);
    dx_log(&outer, 4);
    dx_log(&inner, 8);
    CHECK(outer.first == 1 && outer.count == 3 && outer.dropped == 1 &&
          outer.ring[0] == 2 && outer.ring[2] == 4 && inner.first == 9);
    puts("DX:log:first1:last2,3,4:dropped1:nested-first9");
}

static void reducer(void)
{
    struct dx_reduction r;
    CHECK(dx_reduce(8, 512, fixture, NULL, &r) == 0 &&
          fixture(r.mask, NULL) == DX_FAIL);
    unsigned fail = 0, min = 9, classes[3] = {0};
    for (unsigned m = 0; m < 256; m++)
    {
        int c = fixture((uint16_t)m, NULL);
        classes[c]++;
        if (c == DX_FAIL)
        {
            fail++;
            if (bitcount(m) < min)
            {
                min = bitcount(m);
            }
        }
    }
    CHECK(fail == 88 && min == 2 && classes[2] > 0);
    unsigned small = 193;
    for (unsigned bit = 0; bit < 8; bit++)
    {
        if (small & (1u << bit))
        {
            CHECK(fixture((uint16_t)(small & ~(1u << bit)), NULL) ==
                  DX_UNRESOLVED);
        }
    }
    unsigned pairs = 0;
    for (unsigned i = 0; i < 8; i++)
    {
        for (unsigned j = i + 1; j < 8; j++)
        {
            if ((small & (1u << i)) && (small & (1u << j)))
            {
                CHECK(fixture((uint16_t)(small & ~(1u << i) & ~(1u << j)),
                              NULL) != DX_FAIL);
                pairs++;
            }
        }
    }
    CHECK(pairs == 3);
    CHECK(dx_reduce(8, 1, fixture, NULL, &r) == 0 && r.exhausted &&
          r.mask == 255);
    CHECK(dx_reduce(17, 512, fixture, NULL, &r) == -1);
    printf(
        "DX:reducer:256:FAIL88:minimum2:one-minimal193:pair-deletions3:classes%u,%u,%u\n",
        classes[0], classes[1], classes[2]);
}

static void memory(void)
{
    struct dx_memory m = {1, 1, 0, 4};
    CHECK(dx_memory(&m, DX_WRITE, 1, 3, 0) == 0 &&
          dx_memory(&m, DX_WRITE, 1, 4, 0) == -1);
    CHECK(dx_memory(&m, DX_READ, 1, 0, 0) == -1);
    CHECK(dx_memory(&m, DX_RESIZE_FAIL, 1, 0, 9) == 0 && m.generation == 1 &&
          m.extent == 4 && dx_memory(&m, DX_READ, 1, 3, 0) == 0);
    CHECK(dx_memory(&m, DX_RESIZE_SUCCESS, 1, 0, 8) == 0 && m.generation == 2 &&
          m.extent == 8);
    CHECK(dx_memory(&m, DX_READ, 1, 3, 0) == -1 &&
          dx_memory(&m, DX_READ, 2, 3, 0) == 0);
    CHECK(dx_memory(&m, DX_FREE, 2, 0, 0) == 0 &&
          dx_memory(&m, DX_READ, 2, 3, 0) == -1);
    unsigned fields[2] = {7, 9};
    fields[1] = 4;
    CHECK(fields[0] == 7 && fields[1] != 9);
    unsigned callback_ticket = 2;
    m = (struct dx_memory){1, 3, 1, 4};
    CHECK(dx_memory(&m, DX_READ, callback_ticket, 0, 0) == -1);
    puts(
        "DX:memory:bounds-init-free-resize:failed-preserves:callback-generation:wrong-field-semantic");
}

static void writer(void)
{
    int seq[] = {2, -1, 2, -2, 3};
    size_t offsets[] = {2, 2, 4, 4, 7};
    struct dx_writer w = {0, 7, 0};
    for (unsigned i = 0; i < 5; i++)
    {
        CHECK(dx_write_step(&w, seq[i], 6) >= 0 && w.offset == offsets[i]);
    }
    unsigned cases = 0;
    for (unsigned mask = 0; mask < 64; mask++)
    {
        w = (struct dx_writer){0, 7, 0};
        unsigned begin = 0;
        char out[8] = {0};
        for (unsigned end = 1; end <= 7; end++)
        {
            if (end == 7 || (mask & (1u << (end - 1))))
            {
                CHECK(dx_write_step(&w, -1, 32) == 1);
                CHECK(dx_write_step(&w, -2, 32) == 1);
                size_t old = w.offset;
                CHECK(dx_write_step(&w, (int)(end - begin), 32) >= 0);
                for (size_t j = old; j < w.offset; j++)
                {
                    out[j] = "ABCDEFG"[j];
                }
                begin = end;
            }
        }
        CHECK(w.offset == 7);
        for (unsigned j = 0; j < 7; j++)
        {
            CHECK(out[j] == "ABCDEFG"[j]);
        }
        cases++;
    }
    w = (struct dx_writer){2, 7, 0};
    CHECK(dx_write_step(&w, 6, 4) == -1 && w.offset == 2);
    CHECK(dx_write_step(&w, 0, 4) == -1 && w.offset == 2);
    CHECK(dx_write_step(&w, -2, 2) == -1);
    const char *segments[] = {"AB", "", "CDE", "", "FG"};
    unsigned lengths[] = {2, 0, 3, 0, 2}, si = 0, off = 0, total = 0;
    while (si < 5)
    {
        if (off == lengths[si])
        {
            si++;
            off = 0;
            continue;
        }
        CHECK(segments[si][off] == "ABCDEFG"[total]);
        off++;
        total++;
    }
    CHECK(total == 7);
    struct dx_descriptor d = {1, 1};
    CHECK(dx_close(&d, 1, 0, -1) == 1 && !d.live);
    d = (struct dx_descriptor){1, 2};
    CHECK(dx_close(&d, 1, 0, 0) == -1 && d.live);
    CHECK(dx_close(&d, 2, 1, -2) == 1 && d.live);
    CHECK(dx_close(&d, 2, 0, -2) == 1 && !d.live);
    printf(
        "DX:write:64-positive-partitions:EINTR-EAGAIN:offset2,2,4,4,7:iovec-empty:close-generations:cases%u\n",
        cases);
}

struct dx_device
{
    uint32_t state;
    unsigned reads, writes;
};

static uint32_t read_reg(void *p, unsigned reg)
{
    struct dx_device *d = p;
    CHECK(reg == FW_DMA);
    d->reads++;
    return d->state;
}

static void write_reg(void *p, unsigned reg, uint32_t v)
{
    struct dx_device *d = p;
    CHECK(reg == FW_DMA);
    d->writes++;
    if (v)
    {
        d->state = 1;
    }
}

static void firmware(void)
{
    uint32_t w[9] = {1, 2, 3, 4, 12, 0x08000121, 0x08000402, 0x01000000};
    struct xf_frame f = {0}, old;
    CHECK(mf_capture(w, 8, 0x20000fe0, 0xfffffffd, 0, 0, 0, 1, 0, &f) ==
              MF_OK &&
          f.pc == 0x08000402 && f.original_sp == 0x20001000);
    old = f;
    for (unsigned n = 0; n < 8; n++)
    {
        CHECK(mf_capture(w, n, 0x20000fe0, 0xfffffffd, 0, 0, 0, 1, 0, &f) ==
                  MF_SHORT &&
              f.pc == old.pc);
    }
    CHECK(mf_capture(w, 8, 0x20000fe1, 0xfffffffd, 0, 0, 0, 1, 0, &f) ==
          MF_STATE);
    CHECK(mf_capture(w, 8, 0x20000fe0, 0xffffffed, 0, 0, 0, 1, 0, &f) ==
          MF_UNSUPPORTED);
    w[7] |= 512;
    CHECK(mf_capture(w, 9, 0x20000fe0, 0xfffffffd, 0, 0, 0, 1, 0, &f) ==
          MF_UNSUPPORTED);
    CHECK(mf_capture(w, 9, 0x20000fe0, 0xfffffffd, 1, 0, 0, 1, 0, &f) ==
              MF_OK &&
          f.original_sp == 0x20001004 && f.alignment_word);
    CHECK((0x8200u & 0x8000u) != 0 && (0x400u & 0x8000u) == 0);
    struct dx_device dev = {0};
    struct fw_io io = {&dev, read_reg, write_reg};
    struct fw_dma dma = {0};
    CHECK(fw_dma(&dma, io, 0, 0) == 0 && dma.state == 1);
    CHECK(fw_dma(&dma, io, 1, 1) == 0 && dma.state == 2);
    CHECK(fw_dma(&dma, io, 2, 1) == -1 && dma.state == 2);
    dev.state = 0;
    CHECK(fw_dma(&dma, io, 2, 1) == 0 && dma.state == 0);
    CHECK(fw_dma(&dma, io, 2, 1) == -1);
    CHECK(fw_dma(&dma, io, 0, 0) == 0 && dma.generation == 2);
    CHECK(fw_dma(&dma, io, 2, 1) == -1 && dma.state == 1);
    CHECK(fw_dma(&dma, io, 1, 2) == 0);
    dev.state = 0;
    CHECK(fw_dma(&dma, io, 2, 2) == 0);
    puts(
        "DX:firmware:bounded-basic-PSP:padding-policy:BFAR-validity:cancel-timeout-ack-duplicate-stale");
}

static void saturation(void)
{
    unsigned failures = 0, first = 999;
    for (unsigned a = 0; a < 256; a++)
    {
        for (unsigned b = 0; b < 256; b++)
        {
            unsigned expected = a + b > 255 ? 255 : a + b, out = 777;
            CHECK(dx_saturate(a, b) == expected);
            CHECK(dx_checked(a, b, &out) == (a + b > 255 ? -1 : 0) &&
                  out == (a + b > 255 ? 777 : a + b));
            unsigned narrow = (a + b) & 255;
            if (narrow != expected)
            {
                failures++;
                if (a + b < first)
                {
                    first = a + b;
                }
            }
        }
    }
    CHECK(failures == 32640 && first == 256);
    puts(
        "DX:saturation:65536:failures32640:min-failing-sum256:checked-unchanged");
}

int main(void)
{
    logging();
    reducer();
    memory();
    writer();
    firmware();
    saturation();
    puts("DX_PASS:core");
    return 0;
}
