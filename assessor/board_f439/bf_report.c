#include "bf_report.h"

#include "bf_hw.h"
#include "uart.h"

/* Bounds are poll counts, not durations. At 16 MHz one 8N1 frame at 57600
 * baud lasts 2780 cycles; a chunk of eight frames needs far fewer than
 * BF_CHUNK_POLLS driver steps. */
#define BF_OPEN_POLLS 1000u
#define BF_CHUNK_POLLS 400000u
#define BF_CHUNK_BYTES 8u
#define BF_GENERATION 1u

static struct pu_driver driver;
static struct pu_rx receive_queue;
static struct pu_io io;
static uint32_t token;
static uint32_t baud_rate;
static uint32_t failures;
static uint8_t staged[BF_CHUNK_BYTES];
static unsigned staged_count;

static int open_driver(void)
{
    const struct pu_clock clock = {BF_HSI_HZ, BF_GENERATION, 1u, 1u, 1u, 0u};
    driver = (struct pu_driver){0};
    token = 0u;
    return pu_open(&driver, io, &clock, baud_rate, 0u, BF_OPEN_POLLS);
}

int bf_uart_open(uint32_t baud)
{
    io = pu_f439_io();
    baud_rate = baud;
    receive_queue = (struct pu_rx){0};
    pu_f439_pins();
    return open_driver();
}

/* Logical time is constant: completion is bounded by the poll budget. */
static void send_staged(void)
{
    if (staged_count == 0u)
    {
        return;
    }
    if (token == UINT32_MAX)
    {
        (void)open_driver();
    }
    ++token;
    int result = pu_submit(&driver, io, BF_GENERATION, token, PU_POLL, staged,
                           staged_count, 0u, 1u);
    if (result == PU_OK)
    {
        result = pu_poll(&driver, io, token, 0u, BF_CHUNK_POLLS);
    }
    if (result != PU_OK)
    {
        ++failures;
        (void)open_driver();
    }
    staged_count = 0u;
}

static void put_byte(uint8_t value)
{
    staged[staged_count] = value;
    ++staged_count;
    if (staged_count == BF_CHUNK_BYTES)
    {
        send_staged();
    }
}

void bf_put(const char *text)
{
    for (const char *p = text; *p != '\0'; ++p)
    {
        put_byte((uint8_t)*p);
    }
}

void bf_put_hex(uint32_t value)
{
    static const char digits[] = "0123456789abcdef";
    for (unsigned shift = 32u; shift > 0u; shift -= 4u)
    {
        put_byte((uint8_t)digits[(value >> (shift - 4u)) & 0xFu]);
    }
}

void bf_put_dec(uint32_t value)
{
    char text[11];
    unsigned at = sizeof text - 1u;
    text[at] = '\0';
    do
    {
        --at;
        text[at] = (char)('0' + (char)(value % 10u));
        value /= 10u;
    } while (value != 0u);
    bf_put(&text[at]);
}

void bf_field_hex(const char *name, uint32_t value)
{
    put_byte((uint8_t)' ');
    bf_put(name);
    bf_put("=0x");
    bf_put_hex(value);
}

void bf_field_dec(const char *name, uint32_t value)
{
    put_byte((uint8_t)' ');
    bf_put(name);
    put_byte((uint8_t)'=');
    bf_put_dec(value);
}

void bf_line_end(void)
{
    bf_put("\r\n");
    send_staged();
}

void bf_put_repeated(uint8_t value, uint32_t count)
{
    for (uint32_t i = 0u; i < count; ++i)
    {
        put_byte(value);
    }
    send_staged();
}

int bf_rx_poll(uint8_t *value, uint8_t *error)
{
    struct pu_byte taken;
    (void)pu_rx_poll(&receive_queue, io, 1u);
    if (pu_take(&receive_queue, &taken) != PU_OK)
    {
        return 0;
    }
    *value = taken.value;
    *error = taken.error;
    return 1;
}

uint32_t bf_tx_failures(void)
{
    return failures;
}
