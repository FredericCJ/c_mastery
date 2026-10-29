#include "usb.h"
#if !defined(DG_BIG_ENDIAN) && !defined(DG_STANDALONE)
#include <stdio.h>
#else
int pm_printf(const char *, ...);
int pm_puts(const char *);
#define printf pm_printf
#define puts pm_puts
#endif
#define CHECK(x)                                   \
    do                                             \
    {                                              \
        if (!(x))                                  \
        {                                          \
            printf("DG_USB_CHECK:%d\n", __LINE__); \
            return 1;                              \
        }                                          \
    } while (0)

static int control(struct dg_usb *u, unsigned type, unsigned request,
                   unsigned value, unsigned index, unsigned length,
                   struct dg_control *c)
{
    uint8_t q[8] = {(uint8_t)type,   (uint8_t)request,
                    (uint8_t)value,  (uint8_t)(value >> 8),
                    (uint8_t)index,  (uint8_t)(index >> 8),
                    (uint8_t)length, (uint8_t)(length >> 8)};
    return dg_usb_setup(u, q, c);
}

static int configured(struct dg_usb *u)
{
    struct dg_control c;
    CHECK(control(u, 0, 5, 7, 0, 0, &c) == DG_OK && u->address == 0);
    CHECK(dg_usb_status(u, c.token, 1) == DG_OK && u->address == 7);
    CHECK(control(u, 0, 9, 1, 0, 0, &c) == DG_OK && u->configured == 1);
    CHECK(dg_usb_status(u, c.token, 1) == DG_OK);
    return 0;
}

int main(void)
{
    struct dg_usb u = {0};
    struct dg_control c;
    uint32_t stale;
    for (unsigned length = 0; length <= 40; ++length)
    {
        CHECK(control(&u, 128, 6, 512, 0, length, &c) == DG_OK &&
              c.length == (length < 32 ? length : 32));
        if (length >= 32)
            CHECK(c.bytes[2] == 32 && c.bytes[3] == 0 && c.bytes[20] == 1 &&
                  c.bytes[27] == 129);
    }
    CHECK(control(&u, 128, 6, 1536, 0, 10, &c) == DG_FRAME);
    CHECK(control(&u, 0, 5, 7, 0, 0, &c) == DG_OK && !u.address);
    stale = c.token;
    CHECK(control(&u, 128, 6, 256, 0, 8, &c) == DG_OK && c.length == 8 &&
          c.bytes[7] == 8);
    CHECK(dg_usb_status(&u, stale, 1) == DG_STALE && !u.address);
    CHECK(control(&u, 0, 5, 7, 0, 0, &c) == DG_OK);
    CHECK(dg_usb_status(&u, c.token, 0) == DG_OK && !u.address);
    CHECK(control(&u, 0, 9, 1, 0, 0, &c) == DG_FRAME);
    CHECK(configured(&u) == 0);
    CHECK(control(&u, 0, 5, 8, 0, 0, &c) == DG_FRAME && u.address == 7);
    CHECK(control(&u, 0, 9, 2, 0, 0, &c) == DG_FRAME && u.configured);
    {
        struct dg_session s = {0};
        struct dg_message m = {9, 0, {0}}, out = {99, 0, {0}};
        unsigned hs = 99;
        int app = 99;
        uint8_t bytes[40] = {0, 9, 0, 32};
        CHECK(dg_usb_out(&u, 0, bytes, 9, 1, &s, &out, &hs, &app) ==
                  DG_INVALID &&
              hs == 99 && app == 99);
        CHECK(dg_usb_out(&u, 2, bytes, 8, 1, &s, &out, &hs, &app) ==
              DG_INVALID);
        CHECK(dg_usb_out(&u, 0, 0, 1, 1, &s, &out, &hs, &app) == DG_INVALID);
        for (unsigned i = 0; i < 5; ++i)
            CHECK(dg_usb_out(&u, u.toggle_out, bytes + 8 * i, 8, 1, &s, &out,
                             &hs, &app) == DG_OK &&
                  app == DG_BUSY);
        CHECK(u.out_overflow && s.accepted == 0 && out.sequence == 99);
        CHECK(dg_usb_out(&u, u.toggle_out, 0, 0, 1, &s, &out, &hs, &app) ==
                  DG_OK &&
              app == DG_FRAME && !u.out_overflow && !u.out_used &&
              s.accepted == 0);
        size_t size;
        CHECK(dg_encode(&m, bytes, sizeof bytes, &size) == DG_OK);
        CHECK(dg_usb_out(&u, u.toggle_out, bytes, size, 1, &s, &out, &hs,
                         &app) == DG_OK &&
              app == DG_OK && out.sequence == 9);
        CHECK(control(&u, 128, 8, 0, 0, 1, &c) == DG_OK && c.bytes[0] == 1);
        CHECK(control(&u, 129, 10, 0, 0, 1, &c) == DG_OK && c.bytes[0] == 0);
        CHECK(control(&u, 1, 11, 0, 0, 0, &c) == DG_OK && !u.toggle_out);
        CHECK(control(&u, 130, 0, 0, 1, 2, &c) == DG_OK && !c.bytes[0]);
        CHECK(control(&u, 0, 9, 0, 0, 0, &c) == DG_OK && !u.configured);
        CHECK(dg_usb_out(&u, 0, bytes, size, 1, &s, &out, &hs, &app) == DG_OK &&
              hs == DG_USB_NAK && s.accepted == 1);
        CHECK(control(&u, 0, 9, 1, 0, 0, &c) == DG_OK && u.configured);
    }
    for (unsigned length = 0; length <= 32; ++length)
    {
        struct dg_message m = {(uint16_t)(length + 1), (uint16_t)length, {0}},
                          out = {0};
        for (unsigned i = 0; i < length; ++i)
            m.payload[i] = (uint8_t)(i + 17);
        struct dg_session session = {0};
        uint8_t wire[36];
        size_t n;
        CHECK(dg_encode(&m, wire, sizeof wire, &n) == DG_OK);
        u.out_used = 0;
        u.out_overflow = 0;
        u.toggle_out = 0;
        unsigned hs;
        int app;
        size_t pos = 0;
        while (pos < n)
        {
            size_t part = n - pos > 8 ? 8 : n - pos;
            unsigned toggle = u.toggle_out;
            CHECK(dg_usb_out(&u, toggle, wire + pos, part, 0, &session, &out,
                             &hs, &app) == DG_OK &&
                  hs == DG_USB_NONE && session.accepted == 0);
            CHECK(dg_usb_out(&u, toggle, wire + pos, part, 1, &session, &out,
                             &hs, &app) == DG_OK &&
                  hs == DG_USB_ACK);
            unsigned accepted = session.accepted;
            size_t used = u.out_used;
            unsigned next = u.toggle_out;
            CHECK(dg_usb_out(&u, toggle, wire + pos, part, 1, &session, &out,
                             &hs, &app) == DG_OK &&
                  hs == DG_USB_ACK && app == DG_BUSY &&
                  session.accepted == accepted && u.out_used == used &&
                  u.toggle_out == next);
            pos += part;
        }
        if (n % 8 == 0)
        {
            CHECK(session.accepted == 0);
            CHECK(dg_usb_out(&u, u.toggle_out, 0, 0, 1, &session, &out, &hs,
                             &app) == DG_OK &&
                  app == DG_OK);
        }
        CHECK(session.accepted == 1 && out.sequence == m.sequence &&
              out.length == length);
        CHECK(dg_usb_in_queue(&u, length + 1, &m) == DG_OK);
        pos = 0;
        unsigned packets = 0;
        while (u.in_phase)
        {
            const uint8_t *p, *again;
            size_t part, repeat;
            unsigned toggle, t2;
            CHECK(dg_usb_in_packet(&u, length + 1, &p, &part, &toggle) ==
                      DG_OK &&
                  part <= 8);
            CHECK(dg_usb_in_ack(&u, length + 1, 0) == DG_OK);
            CHECK(dg_usb_in_packet(&u, length + 1, &again, &repeat, &t2) ==
                      DG_OK &&
                  again == p && repeat == part && t2 == toggle);
            for (size_t i = 0; i < part; ++i)
                CHECK(p[i] == wire[pos + i]);
            pos += part;
            ++packets;
            CHECK(dg_usb_in_ack(&u, length + 1, 1) == DG_OK);
        }
        CHECK(pos == n && packets == n / 8 + 1);
    }
    u.toggle_out = 1;
    CHECK(control(&u, 2, 1, 0, 1, 0, &c) == DG_OK &&
          !u.toggle_out); /* clear even when not halted */
    CHECK(control(&u, 2, 3, 0, 1, 0, &c) == DG_OK && u.halt_out);
    struct dg_session session = {0};
    struct dg_message m = {1, 2, {1, 2}}, out = {0};
    unsigned hs;
    int app;
    uint8_t wire[36];
    size_t n;
    CHECK(dg_encode(&m, wire, sizeof wire, &n) == DG_OK);
    CHECK(dg_usb_out(&u, 0, wire, n, 1, &session, &out, &hs, &app) == DG_OK &&
          hs == DG_USB_STALL);
    CHECK(control(&u, 2, 1, 0, 1, 0, &c) == DG_OK && !u.halt_out);
    CHECK(dg_usb_out(&u, 0, wire, n, 1, &session, &out, &hs, &app) == DG_OK &&
          app == DG_OK);
    CHECK(dg_usb_reset(&u, 1) == DG_OK && !u.address && !u.configured);
    CHECK(configured(&u) == 0);
    CHECK(dg_usb_out(&u, 0, wire, n, 1, &session, &out, &hs, &app) == DG_OK &&
          app == DG_STALE && session.accepted == 1);
    CHECK(dg_usb_in_queue(&u, 100, &m) == DG_OK);
    CHECK(dg_usb_in_cancel(&u, 100, 0) == DG_BUSY && u.in_phase == 2);
    CHECK(dg_usb_in_queue(&u, 101, &m) == DG_BUSY);
    CHECK(dg_usb_in_ack(&u, 100, 1) == DG_BUSY);
    CHECK(dg_usb_in_cancel(&u, 100, 1) == DG_OK);
    CHECK(dg_usb_in_queue(&u, 101, &m) == DG_OK);
    CHECK(dg_usb_reset(&u, 0) == DG_BUSY && u.resetting && u.in_phase == 2);
    CHECK(control(&u, 0, 5, 1, 0, 0, &c) == DG_BUSY);
    CHECK(dg_usb_reset(&u, 1) == DG_OK && !u.in_phase);
    CHECK(dg_usb_in_ack(&u, 101, 1) == DG_STALE);
    CHECK(configured(&u) == 0);
    CHECK(dg_usb_in_queue(&u, UINT32_MAX, &m) == DG_OK);
    CHECK(dg_usb_in_cancel(&u, UINT32_MAX, 1) == DG_OK);
    CHECK(dg_usb_in_queue(&u, 1, &m) == DG_STALE);
    u.control_generation = UINT32_MAX;
    CHECK(control(&u, 128, 6, 256, 0, 18, &c) == DG_FULL && !u.pending_control);
    puts("DG_USB_PASS");
    return 0;
}
