#include "specimens.h"
static const struct ni_config config = {0xc0000202u, 0xc0000201u};

static int rejected(unsigned char *p, size_t n)
{
    struct ni_packet out;
    for (unsigned j = 0; j < 1500; j++)
        out.bytes[j] = 0xa5;
    out.length = 777;
    CHECK(ni_echo(&config, p, n, &out) == NI_INVALID && out.length == 777);
    for (unsigned j = 0; j < 1500; j++)
        CHECK(out.bytes[j] == 0xa5);
    return 0;
}

int main(void)
{
    unsigned char p[1500];
    struct ni_view v;
    unsigned rejects = 0;
    for (unsigned n = 0; n < 33; n++)
    {
        ni_test_echo(p, 5);
        CHECK(!rejected(p, n));
        rejects++;
    }
    /* Recompute header checksum after changing a field: testing the semantic
     * branch, not merely rejection of stale checksums. */
    for (unsigned field = 0; field < 13; field++)
    {
        ni_test_echo(p, 5);
        switch (field)
        {
            case 0:
                p[0] = 0x46;
                break;
            case 1:
                p[6] = 0x20;
                break;
            case 2:
                p[7] = 1;
                break;
            case 3:
                p[8] = 0;
                break;
            case 4:
                p[3] = 34;
                break;
            case 5:
                p[3] = 19;
                break;
            case 6:
                p[9] = 17;
                break;
            case 7:
                p[15] = 3;
                break;
            case 8:
                p[19] = 3;
                break;
            case 9:
                p[6] = 0x80;
                break;
            case 10:
                p[6] = 0x60;
                break;
            case 11:
                p[20] = 0;
                break;
            default:
                p[21] = 1;
                break;
        }
        ni_test_recheck(p);
        if (field >= 11)
        {
            p[22] = p[23] = 0;
            ni_test_put16(p + 22, ni_test_sum(p + 20, 13));
        }
        CHECK(!rejected(p, 33));
        rejects++;
    }
    for (unsigned byte = 0; byte < 33; byte++)
        for (unsigned bit = 0; bit < 8; bit++)
        {
            ni_test_echo(p, 5);
            p[byte] ^= (unsigned char)(1u << bit);
            CHECK(!rejected(p, 33));
            rejects++;
        }
    ni_test_echo(p, 65);
    CHECK(!rejected(p, 93));
    rejects++;
    ni_test_header(p, 7, 1, 0, 1, 1);
    CHECK(ni_parse(&config, p, 27, 1, &v) == NI_INVALID);
    rejects++;
    ni_test_header(p, 8, 1, 1480, 1, 1);
    CHECK(ni_parse(&config, p, 28, 1, &v) == NI_INVALID);
    rejects++;
    ni_test_header(p, 0, 1, 8, 0, 1);
    CHECK(ni_parse(&config, p, 20, 1, &v) == NI_INVALID);
    rejects++;
    for (unsigned alignment = 0; alignment < 8; alignment++)
        for (unsigned n = 0; n <= 64; n++)
        {
            ni_test_echo(p + alignment, n);
            struct ni_packet aligned;
            CHECK(ni_echo(&config, p + alignment, n + 28, &aligned) == NI_OK &&
                  aligned.length == n + 28);
            CHECK(aligned.bytes[24] == 0x12 &&
                  ni_test_sum(aligned.bytes + 20, n + 8) == 0);
        }
    /* Constant-checksum collision: interchange two distinct16-bit data words.
     * All packet bounds valid; the harmless success is altered accepted data. */
    ni_test_echo(p, 4);
    unsigned char a = p[28], b = p[29];
    p[28] = p[30];
    p[29] = p[31];
    p[30] = a;
    p[31] = b;
    struct ni_packet out;
    CHECK(ni_echo(&config, p, 32, &out) == NI_OK && out.bytes[28] == 'c');
    CHECK(rejects == 314);
    puts("NI_VALIDATION_PASS rejected=314 unaligned=520 "
         "checksum_collision_observed");
    return 0;
}
