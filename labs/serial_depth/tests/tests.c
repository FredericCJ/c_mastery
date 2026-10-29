#include "serial_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "SD_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int uart(void)
{
    struct sd_uart r;
    struct sd_receive v;
    unsigned i;
    const uint8_t expected[10] = {0, 1, 0, 1, 0, 0, 1, 0, 1, 1};
    memset(&r, 0xa5, sizeof r);
    CHECK(sd_uart(42000000, 115200, 0, 8, 0, 1, 0xa5, &r) == 0);
    CHECK(r.brr == 0x16d && r.divisor == 365 && r.length == 10);
    for (i = 0; i < 10; i++)
    {
        CHECK(r.bits[i] == expected[i]);
    }
    CHECK(sd_uart(42000000, 115200, 1, 8, 0, 1, 0xa5, &r) == 0 &&
          r.brr == 0x2d5);
    CHECK(sd_uart(42000000, 115200, 1, 8, 1, 2, 1, &r) == 0 && r.bits[9] == 1 &&
          r.length == 12);
    CHECK(r.bits[1] == 1 && r.bits[8] == 0);
    CHECK(sd_uart(42000000, 3000000, 0, 8, 0, 1, 0, &r) == SD_INVALID);
    CHECK(sd_uart(42000000, 3000000, 1, 8, 0, 1, 0, &r) == 0);
    CHECK(sd_uart_receive(SD_RXNE | SD_ORE, 0x55, &v) == 0 && !v.valid &&
          v.error == SD_ORE);
    CHECK(sd_uart_receive(SD_RXNE, 0x55, &v) == 0 && v.valid &&
          v.value == 0x55);
    {
        uint8_t decoded = 99;
        CHECK(sd_uart(42000000, 115200, 0, 8, 1, 1, 0x36, &r) == 0);
        CHECK(sd_uart_decode(r.bits, r.length, 8, 1, 1, &decoded) == 0 &&
              decoded == 0x36);
        r.bits[9] ^= 1;
        decoded = 99;
        CHECK(sd_uart_decode(r.bits, r.length, 8, 1, 1, &decoded) == SD_IO &&
              decoded == 99);
    }
    return 0;
}

static int sync_test(void)
{
    struct sd_sync s, b;
    unsigned mode, i;
    for (mode = 0; mode < 4; mode++)
    {
        CHECK(sd_sync(mode >> 1, mode & 1, 0, 1, 1, 0, 0xa5, &s) == 0 &&
              s.edges == 14);
        for (i = 0; i < 14; i++)
        {
            CHECK(s.level[i] == ((mode >> 1) ^ ((i + 1) & 1)));
            CHECK(s.sample[i] == ((i & 1) == (mode & 1)));
        }
    }
    b = s;
    CHECK(sd_sync(0, 0, 1, 1, 0, 1, 0, &s) == SD_BUSY &&
          memcmp(&s, &b, sizeof s) == 0);
    CHECK(sd_sync(0, 0, 1, 0, 1, 0, 0, &s) == SD_INVALID);
    CHECK(sd_sync(0, 0, 1, 1, 0, 0, 0, &s) == 0 && s.edges == 16);
    return 0;
}

static int i2c(void)
{
    struct sd_sdi2c s = {0}, old;
    uint8_t a = 99, out = 99;
    unsigned p = 99;
    CHECK(sd_i2c_begin(&s, 1, 0x50, 7, &a, &out) == 0 && a == SD_START_WRITE &&
          out == 0xa0);
    CHECK(sd_i2c_event(&s, 1, SD_I2C_ACK, 0, &a, &out) == 0 &&
          a == SD_WRITE_REGISTER && out == 7);
    CHECK(sd_i2c_event(&s, 1, SD_I2C_ACK, 0, &a, &out) == 0 &&
          a == SD_RESTART_READ);
    CHECK(sd_i2c_event(&s, 1, SD_I2C_ACK, 0, &a, &out) == 0 &&
          a == SD_READ_NACK_STOP);
    CHECK(sd_i2c_event(&s, 1, SD_I2C_BYTE, 0x5a, &a, &out) == 0 &&
          out == 0x5a && !s.active);
    CHECK(sd_i2c_begin(&s, 1, 0x50, 7, &a, &out) == SD_STALE);
    CHECK(sd_i2c_begin(&s, 2, 0x50, 7, &a, &out) == 0);
    old = s;
    CHECK(sd_i2c_event(&s, 1, SD_I2C_ACK, 0, &a, &out) == SD_STALE &&
          memcmp(&old, &s, sizeof s) == 0);
    CHECK(sd_i2c_event(&s, 2, SD_I2C_ARB, 0, &a, &out) == SD_ARBITRATION_LOST &&
          a == SD_RELEASE);
    CHECK(sd_i2c_clear(1, 0, 3, &p) == 0 && p == 3);
    CHECK(sd_i2c_clear(1, 0, 10, &p) == SD_RESET && p == 9);
    CHECK(sd_i2c_clear(0, 0, 3, &p) == SD_RESET && p == 0);
    return 0;
}

static int spi(void)
{
    struct sd_spi s = {0};
    uint8_t l[16], p[16], m[16], r;
    unsigned mode, i;
    for (mode = 0; mode < 4; mode++)
    {
        CHECK(sd_spi_edge(mode, 0xa6, 0x3c, l, p, m, &r) == 0 && r == 0x3c);
        for (i = 0; i < 16; i++)
        {
            CHECK(p[i] == ((i & 1) == (mode & 1)));
            CHECK(m[i] == ((0xa6u >> (7 - i / 2)) & 1));
        }
    }
    CHECK(sd_spi_begin(&s, 1, 0, 1) == 0);
    CHECK(sd_spi_poll(&s, 1, 1, 1, 1, 0x3c) == SD_BUSY && s.position == 1 &&
          s.state != 0);
    CHECK(sd_spi_poll(&s, 1, 1, 0, 0, 0) == SD_DONE && s.rx[0] == 0x3c);
    CHECK(sd_spi_begin(&s, 2, 3, 16) == 0);
    CHECK(sd_spi_cancel(&s, 2, 0) == SD_BUSY && s.state == 3);
    CHECK(sd_spi_poll(&s, 2, 1, 1, 0, 0) == SD_BUSY);
    CHECK(sd_spi_cancel(&s, 2, 1) == SD_DONE);
    CHECK(sd_spi_begin(&s, 2, 0, 1) == SD_STALE);
    return 0;
}

static int electrical(void)
{
    int d = 77;
    CHECK(sd_electrical(0, -6000, 1, 1, 0, &d) == 0 && d == 1);
    CHECK(sd_electrical(0, 6000, 1, 1, 0, &d) == 0 && d == 0);
    CHECK(sd_electrical(0, 0, 1, 1, 0, &d) == 0 && d == -1);
    CHECK(sd_electrical(1, 199, 1, 2, 0, &d) == 0 && d == -1);
    CHECK(sd_electrical(1, 200, 1, 2, 0, &d) == 0 && d == 1);
    CHECK(sd_electrical(1, 200, 1, 2, 1, &d) == 0 && d == 0);
    d = 77;
    CHECK(sd_electrical(1, 200, 2, 2, 0, &d) == SD_INVALID && d == 77);
    return 0;
}

static int rs485(void)
{
    struct sd_rs485 s = {0};
    uint8_t b[2] = {0xa5, 0x5a};
    CHECK(sd_485_begin(&s, 1, b, 2, 0) == 0);
    CHECK(sd_485_poll(&s, 1, 599, 1, 0) == SD_BUSY && s.position == 0);
    CHECK(sd_485_poll(&s, 1, 600, 1, 0) == SD_BUSY && s.position == 1);
    CHECK(sd_485_poll(&s, 1, 100600, 1, 0) == SD_BUSY && s.position == 2 &&
          s.phase == 2);
    CHECK(sd_485_poll(&s, 1, 100601, 1, 0) == SD_BUSY && s.phase == 2);
    CHECK(sd_485_poll(&s, 1, 200600, 1, 1) == SD_BUSY && s.due == 202800);
    CHECK(sd_485_poll(&s, 1, 202799, 1, 1) == SD_BUSY);
    CHECK(sd_485_poll(&s, 1, 202800, 1, 1) == SD_DONE &&
          s.events[0] == SD_485_TXDONE);
    CHECK(sd_485_receive(&s, SD_485_RX, 0x33) == 0 && s.rx[0] == 0x33);
    CHECK(sd_485_receive(&s, SD_485_FRAMING, 0) == 0 && s.rxcount == 1);
    CHECK(sd_485_begin(&s, 2, b, 2, 202800) == 0);
    CHECK(sd_485_cancel(&s, 2, 202801, 0) == SD_BUSY && s.phase == 4);
    CHECK(sd_485_cancel(&s, 2, 202802, 1) == SD_BUSY && s.phase == 3);
    CHECK(sd_485_poll(&s, 2, 205002, 0, 0) == SD_DONE &&
          s.events[3] == SD_485_ABORTED);
    {
        uint8_t event = 99, value = 99;
        CHECK(sd_485_take(&s, &event, &value) == 0 && event == SD_485_TXDONE &&
              value == 0);
        CHECK(sd_485_take(&s, &event, &value) == 0 && event == SD_485_RX &&
              value == 0x33);
        CHECK(sd_485_take(&s, &event, &value) == 0 && event == SD_485_FRAMING &&
              value == 0);
        CHECK(sd_485_take(&s, &event, &value) == 0 && event == SD_485_ABORTED &&
              value == 0);
        event = 99;
        value = 99;
        CHECK(sd_485_take(&s, &event, &value) == SD_FULL && event == 99 &&
              value == 99);
    }
    return 0;
}

static int can(void)
{
    struct sd_can a = {0x123, 0, 0, 1, {0xa5}}, b = {0},
                  ext = {0x123u << 18, 1, 0, 0, {0}},
                  remote = {0x123, 0, 1, 1, {0}};
    uint8_t bits[160], bad[160];
    size_t n = 0, k;
    int w = 9;
    CHECK(sd_can_arbitration(&a, &remote, &w) == 0 && w == -1);
    CHECK(sd_can_arbitration(&remote, &ext, &w) == 0 && w == -1);
    CHECK(sd_can_encode(&a, bits, 160, &n) == 0 && bits[0] == 0);
    CHECK(sd_can_decode(bits, n, &b) == 0 && b.identifier == a.identifier &&
          b.length == 1 && b.data[0] == 0xa5);
    for (k = 0; k < n; k++)
    {
        bad[k] =
            bits[k]; /* Last CRC sequence bit is immediately before ten fixed
                        trailer bits, or before its mandatory stuff bit. */
    }
    bad[n - 11] ^= 1;
    CHECK(sd_can_decode(bad, n, &b) != 0);
    for (k = 0; k < n; k++)
    {
        bad[k] = bits[k];
    }
    bad[n - 2] = 0;
    CHECK(sd_can_decode(bad, n, &b) != 0);
    CHECK(sd_can_encode(&a, bits, 1, &n) == SD_FULL);
    a.extended = 1;
    a.identifier = 0x1234567;
    CHECK(sd_can_encode(&a, bits, 160, &n) == 0);
    CHECK(sd_can_decode(bits, n, &b) == 0 && b.identifier == a.identifier);
    return 0;
}

static int bxcan(void)
{
    struct sd_timing t;
    struct sd_fifo f = {{0}, 0, 0, 0};
    uint32_t v;
    unsigned accept;
    {
        struct sd_mailbox m = {0};
        uint8_t result = 99;
        CHECK(sd_can_submit(&m, 1) == 0 &&
              sd_can_take(&m, 1, &result) == SD_BUSY && result == 99);
        CHECK(sd_can_complete(&m, 1, 1) == SD_BUSY && m.state == 1);
        CHECK(sd_can_complete(&m, 1, 0) == 0 &&
              sd_can_take(&m, 1, &result) == 0 && result == 0);
        CHECK(sd_can_take(&m, 1, &result) == SD_STALE &&
              sd_can_submit(&m, 2) == 0);
        CHECK(sd_can_complete(&m, 1, 0) == SD_STALE && m.state == 1);
    }
    CHECK(sd_can_timing(42000000, 6, 11, 2, 1, &t) == 0 && t.rate == 500000 &&
          t.btr == 0x1a0005);
    CHECK(sd_can_timing(42000000, 6, 11, 1, 2, &t) == SD_INVALID);
    CHECK(sd_can_filter(0x12f, 0x120, 0x7f0, &accept) == 0 && accept == 1);
    CHECK(sd_can_filter(0x130, 0x120, 0x7f0, &accept) == 0 && accept == 0);
    CHECK(sd_can_push(&f, 10) == 0 && sd_can_push(&f, 20) == 0 &&
          sd_can_push(&f, 30) == 0 && sd_can_push(&f, 40) == 0);
    CHECK(f.overrun && sd_can_pop(&f, &v) == 0 && v == 10);
    CHECK(sd_can_pop(&f, &v) == 0 && v == 20);
    CHECK(sd_can_pop(&f, &v) == 0 && v == 40);
    f.locked = 1;
    CHECK(sd_can_push(&f, 1) == 0 && sd_can_push(&f, 2) == 0 &&
          sd_can_push(&f, 3) == 0 && sd_can_push(&f, 4) == SD_FULL &&
          f.items[2] == 3);
    return 0;
}

static int fd(void)
{
    struct sd_fd f, old;
    CHECK(sd_fd(9, 1, 0, 1, 0, 1, &f) == 0 && f.length == 12 &&
          f.crc_width == 17);
    CHECK(sd_fd(10, 1, 1, 0, 0, 1, &f) == 0 && f.length == 16 &&
          f.crc_width == 17);
    CHECK(sd_fd(15, 1, 1, 0, 0, 1, &f) == 0 && f.length == 64 &&
          f.crc_width == 21 && f.spi_bytes == 68);
    old = f;
    CHECK(sd_fd(9, 1, 2, 0, 0, 1, &f) == SD_INVALID &&
          memcmp(&f, &old, sizeof f) == 0);
    CHECK(sd_fd(0, 1, 0, 0, 0, 1, &f) == 0 && f.length == 0);
    CHECK(sd_fd(1, 1, 0, 0, 0, 0, &f) == SD_UNSUPPORTED);
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "uart"))
    {
        return uart();
    }
    if (!strcmp(argv[1], "sync"))
    {
        return sync_test();
    }
    if (!strcmp(argv[1], "i2c"))
    {
        return i2c();
    }
    if (!strcmp(argv[1], "spi"))
    {
        return spi();
    }
    if (!strcmp(argv[1], "electrical"))
    {
        return electrical();
    }
    if (!strcmp(argv[1], "rs485"))
    {
        return rs485();
    }
    if (!strcmp(argv[1], "can"))
    {
        return can();
    }
    if (!strcmp(argv[1], "bxcan"))
    {
        return bxcan();
    }
    if (!strcmp(argv[1], "fd"))
    {
        return fd();
    }
    return 2;
}
