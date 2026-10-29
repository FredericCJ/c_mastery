#include "retry.h"
#include <string.h>
void sr_note(const char *);
#define CHECK(x)                           \
    do                                     \
    {                                      \
        if (!(x))                          \
        {                                  \
            sr_note("SR_ASSERT:" #x "\n"); \
            return 41;                     \
        }                                  \
    } while (0)

static int stretch(void)
{
    struct sr_timer t;
    uint32_t d = 99;
    unsigned count = 0;
    for (unsigned start = 0; start < 256; start++)
    {
        for (unsigned n = 1; n < 128; n++)
        {
            CHECK(sr_timer_begin(&t, 8, start, n) == SR_OK);
            CHECK(sr_timer_step(&t, (start + n - 1) % 256, &d) == SR_OK &&
                  t.remaining == 1);
            CHECK(sr_timer_step(&t, (start + n) % 256, &d) == SR_TIMEOUT &&
                  t.remaining == 0);
            count++;
        }
    }
    CHECK(count == 32512);
    CHECK(sr_timer_begin(&t, 8, 250, 20) == SR_OK);
    struct sr_timer old = t;
    CHECK(sr_timer_step(&t, 122, &d) == SR_AMBIGUOUS &&
          !memcmp(&t, &old, sizeof t));
    struct sr_byte s;
    CHECK(sr_byte_begin(&s, 0x80, 8, 250, 30, 5) == SR_OK);
    CHECK(sr_byte_sample(&s, 251, 0, 1, 1) == SR_PENDING && s.bit.bit == 0);
    CHECK(sr_byte_sample(&s, 255, 0, 1, 1) == SR_PENDING && s.bit.bit == 0);
    CHECK(sr_byte_sample(&s, 0, 1, 1, 1) == SR_TIMEOUT && s.bit.bit == 0);
    CHECK(sr_byte_begin(&s, 0x80, 8, 250, 30, 5) == SR_OK);
    CHECK(sr_byte_sample(&s, 251, 0, 1, 1) == SR_PENDING);
    CHECK(sr_byte_sample(&s, 254, 1, 1, 1) == SR_PENDING && s.bit.bit == 1);
    CHECK(sr_byte_sample(&s, 255, 0, 0, 0) == SR_PENDING);
    CHECK(sr_byte_sample(&s, 2, 1, 0, 1) == SR_PENDING && s.bit.bit == 2);
    CHECK(sr_byte_begin(&s, 0x80, 8, 250, 10, 5) == SR_OK);
    CHECK(sr_byte_sample(&s, 251, 1, 0, 1) == SR_LOST &&
          s.bit.state == S_I2C_LOST);
    CHECK(sr_byte_begin(&s, 0, 8, 0, 10, 5) == SR_OK);
    for (unsigned i = 0; i < 8; i++)
    {
        CHECK(sr_byte_sample(&s, i, 1, 0, 1) == SR_PENDING);
    }
    CHECK(sr_byte_sample(&s, 8, 1, 1, 1) == SR_NACK);
    CHECK(sr_byte_begin(&s, 0, 8, 0, 10, 5) == SR_OK);
    CHECK(sr_byte_sample(&s, 10, 1, 0, 1) == SR_TIMEOUT && s.bit.bit == 0);
    sr_note("SR_VALUES stretch 32512 5 10 128\nSR_STRETCH_PASS\n");
    return 0;
}

static int transaction(void)
{
    const unsigned addresses[] = {0, 1, 0x50, 127};
    const unsigned registers[] = {0, 0x80, 255};
    unsigned count = 0;
    for (unsigned a = 0; a < 4; a++)
    {
        for (unsigned reg = 0; reg < 3; reg++)
        {
            struct sr_txn t;
            uint32_t now = 65530;
            CHECK(sr_txn_begin(&t, 7, (uint8_t)addresses[a],
                               (uint8_t)registers[reg], 16, now,
                               100) == SR_OK &&
                  t.action == SD_START_WRITE && t.wire == addresses[a] * 2);
            for (unsigned phase = 0; phase < 3; phase++)
            {
                for (unsigned bit = 0; bit < 8; bit++)
                {
                    unsigned level;
                    CHECK(s_i2c_drive(&t.bit, &level) == S_OK);
                    now = (now + 1) % 65536;
                    CHECK(sr_txn_bit(&t, 7, now, 0, level) == SR_PENDING &&
                          t.bit.bit == bit);
                    CHECK(sr_txn_bit(&t, 7, now, 1, level) == SR_PENDING);
                }
                now = (now + 1) % 65536;
                CHECK(sr_txn_bit(&t, 7, now, 1, 0) ==
                      (phase == 2 ? SR_READY : SR_PENDING));
                if (phase == 0)
                {
                    CHECK(t.action == SD_WRITE_REGISTER &&
                          t.wire == registers[reg]);
                }
                if (phase == 1)
                {
                    CHECK(t.action == SD_RESTART_READ &&
                          t.wire == addresses[a] * 2 + 1);
                }
            }
            CHECK(t.action == SD_READ_NACK_STOP && t.bus.active);
            uint8_t out = 99;
            struct sr_txn old = t;
            CHECK(sr_txn_read(&t, 6, now, 0x5a, &out) == SR_STALE &&
                  out == 99 && !memcmp(&t, &old, sizeof t));
            CHECK(sr_txn_read(&t, 7, (now + 1) % 65536, 0x5a, &out) ==
                      SR_DONE &&
                  out == 0x5a && t.action == SD_STOP && !t.bus.active);
            count++;
        }
    }
    struct sr_txn t;
    CHECK(sr_txn_begin(&t, 1, 0x50, 0, 8, 0, 20) == SR_OK);
    CHECK(sr_txn_bit(&t, 1, 1, 1, 0) == SR_LOST && t.action == SD_RELEASE &&
          !t.bus.active);
    CHECK(sr_txn_begin(&t, 1, 0, 0, 8, 0, 20) == SR_OK);
    for (unsigned i = 0; i < 8; i++)
    {
        CHECK(sr_txn_bit(&t, 1, i, 1, 0) == SR_PENDING);
    }
    CHECK(sr_txn_bit(&t, 1, 8, 1, 1) == SR_NACK && t.action == SD_STOP);
    CHECK(sr_txn_begin(&t, 1, 0, 0, 8, 0, 20) == SR_OK);
    CHECK(sr_txn_bit(&t, 1, 20, 1, 0) == SR_TIMEOUT && t.action == SD_STOP &&
          t.bit.bit == 0);
    CHECK(count == 12);
    sr_note("SR_VALUES transaction 12 27 1 6\nSR_TRANSACTION_PASS\n");
    return 0;
}

static int retry(void)
{
    struct sr_retry r;
    CHECK(sr_retry_begin(&r, 8, 250, 60, 100, 0x10, 3, 3, 5) == SR_OK);
    CHECK(sr_retry_poll(&r, 251, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 253, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 254, 0, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 255, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 1, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 2, 1, 1, 1) == SR_READY && r.attempts == 1 &&
          r.token == 101);
    uint8_t device = 9, reply = 88;
    CHECK(pi_device(0x10, 1, &device, &reply) == PI_TIMEOUT && device == 9 &&
          reply == 88);
    struct sr_retry old = r;
    CHECK(sr_retry_complete(&r, 100, 3, PI_TIMEOUT, 1) == SR_STALE &&
          !memcmp(&r, &old, sizeof r));
    CHECK(sr_retry_complete(&r, 101, 3, PI_TIMEOUT, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 4, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 7, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 8, 1, 1, 1) == SR_READY && r.attempts == 2);
    CHECK(pi_device(0x10, 0, &device, &reply) == PI_OK && reply == 9);
    CHECK(sr_retry_complete(&r, 102, 9, PI_OK, 1) == SR_DONE);
    CHECK(sr_retry_begin(&r, 8, 0, 30, 1, 0x11, 3, 1, 0) == SR_OK);
    CHECK(sr_retry_poll(&r, 0, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 1, 1, 1, 1) == SR_READY);
    CHECK(pi_device(0x11, 1, &device, &reply) == PI_TIMEOUT && device == 10);
    CHECK(sr_retry_complete(&r, 2, 2, PI_TIMEOUT, 1) == SR_UNCERTAIN &&
          r.attempts == 1);
    CHECK(sr_retry_poll(&r, 3, 1, 1, 1) == SR_UNCERTAIN);
    CHECK(sr_retry_begin(&r, 8, 0, 30, 1, 0x11, 1, 1, 0) == SR_OK);
    CHECK(sr_retry_poll(&r, 0, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 1, 1, 1, 1) == SR_READY);
    CHECK(sr_retry_complete(&r, 2, 2, PI_ADDRESS_NACK, 0) == SR_EXHAUSTED);
    CHECK(sr_retry_begin(&r, 8, 0, 5, 1, 0x10, 3, 1, 0) == SR_OK);
    CHECK(sr_retry_poll(&r, 0, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 1, 1, 1, 1) == SR_READY);
    CHECK(sr_retry_complete(&r, 2, 2, PI_ARBITRATION, 0) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 3, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 4, 1, 1, 1) == SR_READY);
    CHECK(sr_retry_complete(&r, 3, 5, PI_OK, 1) == SR_TIMEOUT);
    CHECK(sr_retry_begin(&r, 8, 0, 30, UINT32_MAX - 1, 0x10, 3, 1, 0) ==
          SR_INVALID);
    CHECK(sr_retry_begin(&r, 8, 0, 30, 1, 0x10, 3, 2, 0) == SR_OK);
    CHECK(sr_retry_poll(&r, 0, 1, 1, 0) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 10, 1, 1, 0) == SR_PENDING && r.attempts == 0);
    CHECK(sr_retry_poll(&r, 11, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 13, 1, 1, 1) == SR_READY);
    /* Actual scheduler -> byte/transaction producer -> classified retry. */
    CHECK(sr_retry_begin(&r, 16, 65530, 100, 10, 0x10, 2, 1, 1) == SR_OK);
    CHECK(sr_retry_poll(&r, 65530, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 65531, 1, 1, 1) == SR_READY);
    struct sr_txn tx;
    CHECK(sr_txn_begin(&tx, r.token, 0x50, 7, 16, 65531, 99) == SR_OK);
    CHECK(sr_txn_bit(&tx, r.token, 65532, 1, 0) == SR_LOST &&
          tx.action == SD_RELEASE);
    CHECK(sr_retry_complete(&r, r.token, 65532, PI_ARBITRATION, 0) ==
          SR_PENDING);
    CHECK(sr_retry_poll(&r, 65533, 1, 1, 0) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 65534, 1, 1, 1) == SR_PENDING);
    CHECK(sr_retry_poll(&r, 65535, 1, 1, 1) == SR_READY);
    CHECK(sr_txn_begin(&tx, r.token, 0x50, 7, 16, 65535, r.timer.remaining) ==
          SR_OK);
    uint32_t now = 65535;
    for (unsigned phase = 0; phase < 3; phase++)
    {
        for (unsigned bit = 0; bit < 8; bit++)
        {
            unsigned level;
            CHECK(s_i2c_drive(&tx.bit, &level) == S_OK);
            now = (now + 1) % 65536;
            CHECK(sr_txn_bit(&tx, r.token, now, 1, level) == SR_PENDING);
        }
        now = (now + 1) % 65536;
        CHECK(sr_txn_bit(&tx, r.token, now, 1, 0) ==
              (phase == 2 ? SR_READY : SR_PENDING));
    }
    CHECK(sr_txn_read(&tx, r.token, (now + 1) % 65536, 0x5a, &reply) ==
              SR_DONE &&
          reply == 0x5a);
    CHECK(sr_retry_complete(&r, r.token, (now + 1) % 65536, PI_OK, 1) ==
              SR_DONE &&
          r.attempts == 2);
    sr_note("SR_COMPOSE_PASS actual_byte_transaction_retry\n");
    sr_note("SR_VALUES retry 2 10 1 5\nSR_RETRY_PASS\n");
    return 0;
}

int sr_test(unsigned which)
{
    return which == 0   ? stretch()
           : which == 1 ? transaction()
           : which == 2 ? retry()
                        : 41;
}
