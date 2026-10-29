#include "board_input.h"
#include "test.h"

struct ac_bus
{
    unsigned char wire[3];
    unsigned available, position, reads, error;
};

static uint32_t read_reg(void *p, uint32_t address)
{
    struct ac_bus *b = p;
    if (address == 0x40004400u)
    {
        return b->error ? 2u : (b->position < b->available ? 32u : 0u);
    }
    CHECK(address == 0x40004404u);
    ++b->reads;
    if (b->error)
    {
        return 0;
    }
    return b->wire[b->position++];
}

static void write_reg(void *p, uint32_t a, uint32_t v)
{
    (void)p;
    (void)a;
    (void)v;
    CHECK(0);
}

static void sync_bus(void *p)
{
    (void)p;
}

int main(void)
{
    struct ac_bus b = {0};
    CHECK(ar_encode(1, (struct ar_record){513}, b.wire, 3) == AR_OK);
    struct fh_io io = {&b, read_reg, write_reg, sync_bus};
    unsigned char bytes[3];
    CHECK(ar_f439_read(io, bytes, 3).state == AR_PENDING);
    struct ar_reader parser = {{0}, 0, 1};
    struct ar_record value = {99};
    b.available = 1;
    struct ar_result r = ar_f439_read(io, bytes, 3);
    CHECK(r.count == 1 && r.state == AR_OK &&
          ar_feed(&parser, bytes, 1, &value) == AR_PENDING);
    b.available = 3;
    r = ar_f439_read(io, bytes, 3);
    CHECK(r.count == 2 && r.state == AR_OK &&
          ar_feed(&parser, bytes, 2, &value) == AR_OK && value.value == 513);
    b.error = 1;
    r = ar_f439_read(io, bytes, 3);
    CHECK(r.state == AR_FAILED && r.count == 0 && b.reads == 4);
    puts("PASS actual frozen F439 receive driver with scripted SR/DR bus and "
         "shared parser; no peripheral execution");
}
