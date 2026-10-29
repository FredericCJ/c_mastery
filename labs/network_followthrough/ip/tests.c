#include "specimens.h"
static const struct ni_config config = {0xc0000202u, 0xc0000201u};

static int checksums(void)
{
    unsigned char bytes[257];
    for (unsigned i = 0; i < 257; i++)
    {
        bytes[i] = 255;
    }
    for (unsigned n = 0; n <= 257; n++)
    {
        for (unsigned split = 0; split <= n; split++)
        {
            struct s_checksum s = {0};
            uint16_t actual = 7, again = 9;
            CHECK(!s_checksum_add(&s, bytes, split));
            CHECK(!s_checksum_finish(&s, &again));
            CHECK(!s_checksum_add(&s, bytes + split, n - split));
            CHECK(!s_checksum_finish(&s, &actual));
            CHECK(actual == ni_test_sum(bytes, n));
            CHECK(!s_checksum_finish(&s, &again) && again == actual);
        }
    }
    struct s_checksum s = {0};
    uint16_t value = 0;
    unsigned char odd = 0x61;
    CHECK(!s_checksum_add(&s, &odd, 1));
    CHECK(!s_checksum_finish(&s, &value) && value == 0x9eff);
    odd = 0x62;
    CHECK(!s_checksum_add(&s, &odd, 1));
    CHECK(!s_checksum_finish(&s, &value) && value == 0x9e9d);
    s = (struct s_checksum){0};
    for (unsigned i = 0; i < 510; i++)
    {
        CHECK(!s_checksum_add(&s, bytes, 257));
    }
    CHECK(!s_checksum_add(&s, bytes, 6));
    CHECK(!s_checksum_finish(&s, &value) && value == 0);
    /* 131076 all-FF bytes exceed an unbounded32-bit accumulator's safe count. */
    struct s_checksum bad = {65536, 0, 0};
    value = 0x1234;
    CHECK(s_checksum_finish(&bad, &value) == S_INVALID && value == 0x1234);
    puts("NI_CHECKSUM_PASS splits=33411 long_bytes=131076");
    return 0;
}

static int ipv4(void)
{
    unsigned char input[100];
    struct ni_packet out;
    struct ni_view v;
    for (unsigned n = 0; n <= 64; n++)
    {
        ni_test_echo(input, n);
        for (unsigned i = n + 28; i < 100; i++)
        {
            input[i] = 0x7b;
        }
        CHECK(ni_parse(&config, input, 100, 0, &v) == NI_OK &&
              v.length == n + 8);
        CHECK(ni_echo(&config, input, 100, &out) == NI_OK &&
              out.length == n + 28);
        CHECK(out.bytes[24] == 0x12 && out.bytes[25] == 0x34 &&
              out.bytes[27] == 1);
        CHECK(out.bytes[20] == 0 && out.bytes[21] == 0 && out.bytes[8] == 64);
        CHECK(out.bytes[6] == 0x40 && out.bytes[7] == 0 && out.bytes[4] == 0 &&
              out.bytes[5] == 0);
        CHECK(ni_test_sum(out.bytes, 20) == 0 &&
              ni_test_sum(out.bytes + 20, n + 8) == 0);
        for (unsigned i = 0; i < n; i++)
        {
            CHECK(out.bytes[28 + i] == (unsigned char)(0x61 + i));
        }
    }
    ni_test_echo(input, 5);
    out.length = 999;
    out.bytes[0] = 0x55;
    CHECK(ni_parse(&config, input, 32, 0, &v) == NI_INVALID);
    input[10] ^= 1;
    CHECK(ni_parse(&config, input, 100, 0, &v) == NI_INVALID);
    CHECK(ni_echo(&config, input, 100, &out) == NI_INVALID &&
          out.length == 999 && out.bytes[0] == 0x55);
    puts("NI_IP_PASS echo_lengths=65 padded_and_rejected");
    return 0;
}

static int reassembly(void)
{
    unsigned char p[36];
    struct ni_reassembly s;
    struct ni_packet out;
    unsigned index = 9;
    CHECK(ni_init(&s, config, 2, NI_STRICT, 100, 60000) == NI_OK);
    ni_test_header(p, 8, 7, 0, 1, 1);
    for (unsigned j = 0; j < 8; j++)
    {
        p[20 + j] = (unsigned char)j;
    }
    CHECK(ni_receive(&s, p, 28, 100, &index) == NI_WAIT && index == 0);
    ni_test_header(p, 8, 7, 16, 0, 1);
    for (unsigned j = 0; j < 8; j++)
    {
        p[20 + j] = (unsigned char)(16 + j);
    }
    CHECK(ni_receive(&s, p, 28, 101, &index) == NI_WAIT);
    ni_test_header(p, 8, 7, 8, 1, 1);
    for (unsigned j = 0; j < 8; j++)
    {
        p[20 + j] = (unsigned char)(8 + j);
    }
    CHECK(ni_receive(&s, p, 28, 102, &index) == NI_OK);
    CHECK(ni_take(&s, index, &out) == NI_OK && out.length == 44);
    for (unsigned j = 0; j < 24; j++)
    {
        CHECK(out.bytes[20 + j] == j);
    }
    CHECK(ni_test_sum(out.bytes, 20) == 0 && out.bytes[6] == 0 &&
          out.bytes[7] == 0);
    CHECK(ni_receive(&s, p, 28, 103, &index) == NI_CONFLICT);
    CHECK(ni_tick(&s, 60100) == NI_OK && s.slot[0].state == NI_FREE);
    CHECK(ni_tick(&s, 60099) == NI_INVALID && s.last == 60100);
    CHECK(ni_init(&s, config, 2, NI_STRICT, 0, 60000) == NI_OK);
    ni_test_header(p, 8, 1, 0, 1, 1);
    CHECK(ni_receive(&s, p, 28, 0, &index) == NI_WAIT);
    ni_test_header(p, 8, 1, 8, 0, 6);
    CHECK(ni_receive(&s, p, 28, 1, &index) == NI_WAIT && index == 1);
    ni_test_header(p, 8, 1, 0, 1, 1);
    CHECK(ni_receive(&s, p, 28, 2, &index) == NI_CONFLICT);
    CHECK(s.slot[0].state == NI_POISON && s.slot[1].state == NI_ACTIVE);
    ni_test_header(p, 8, 2, 0, 1, 1);
    CHECK(ni_receive(&s, p, 28, 3, &index) == NI_BUSY);
    CHECK(ni_init(&s, config, 1, NI_IDENTICAL, 0, 60000) == NI_OK);
    ni_test_header(p, 8, 7, 0, 1, 1);
    CHECK(ni_receive(&s, p, 28, 0, &index) == NI_WAIT);
    CHECK(ni_receive(&s, p, 28, 59999, &index) == NI_WAIT &&
          s.slot[0].expiry == 60000);
    CHECK(ni_tick(&s, 60000) == NI_OK && s.slot[0].state == NI_FREE);
    CHECK(ni_receive(&s, p, 28, 60000, &index) == NI_WAIT);
    p[20] ^= 1;
    CHECK(ni_receive(&s, p, 28, 60001, &index) == NI_CONFLICT);
    CHECK(ni_init(&s, config, 2, 0, UINT64_MAX - 59999, 60000) == NI_INVALID);
    puts("NI_REASSEMBLY_PASS coverage_identity_overlap_absolute_expiry");
    return 0;
}

int main(void)
{
    if (checksums() || ipv4() || reassembly())
    {
        return 1;
    }
    puts("NI_PASS");
    return 0;
}
