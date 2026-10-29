/* Adversarial provider contract probe; actual PuDriver integration is separate.
 */
#include "uart_adapter.h"
#include <stdio.h>
#define CHECK(x)                         \
    do                                   \
    {                                    \
        if (!(x))                        \
        {                                \
            puts("PL_STATUS_CHECK:" #x); \
            return 1;                    \
        }                                \
    } while (0)
static int response;

int pu_open(struct pu_driver *d, struct pu_io io, const struct pu_clock *c,
            uint32_t b, unsigned o, unsigned p)
{
    (void)d;
    (void)io;
    (void)c;
    (void)b;
    (void)o;
    (void)p;
    return PU_OK;
}

int pu_submit(struct pu_driver *d, struct pu_io io, uint32_t g, uint32_t t,
              unsigned k, const uint8_t *p, unsigned n, uint32_t now,
              uint32_t b)
{
    (void)d;
    (void)io;
    (void)g;
    (void)t;
    (void)k;
    (void)p;
    (void)n;
    (void)now;
    (void)b;
    return PU_OK;
}

int pu_step(struct pu_driver *d, struct pu_io io, uint32_t t, uint32_t now)
{
    (void)d;
    (void)io;
    (void)t;
    (void)now;
    return response;
}

int pu_cancel(struct pu_driver *d, struct pu_io io, uint32_t t)
{
    (void)d;
    (void)io;
    (void)t;
    return response;
}

int main(void)
{
    static const int provider[] = {
        PU_OK, PU_INVALID,   PU_BUSY,          PU_TIMEOUT, PU_STALE, PU_FULL,
        PU_IO, PU_CANCELLED, PU_UNIMPLEMENTED, 99,         -1};
    static const int mapped[] = {
        PL_OK, PL_INVALID,   PL_BUSY,    PL_TIMEOUT, PL_STALE,  PL_INVALID,
        PL_IO, PL_CANCELLED, PL_INVALID, PL_INVALID, PL_INVALID};
    for (unsigned i = 0; i < sizeof provider / sizeof provider[0]; i++)
    {
        struct pl_uart uart = {0};
        struct pl_service service = {0};
        struct pl_transport transport = pl_uart_transport(&uart);
        struct pl_result out = {77, 77, 77};
        CHECK(pl_send(&service, transport, 4, 5, 1, 0, 10) == PL_OK);
        response = provider[i];
        int r = pl_poll(&service, transport, 1, 1, &out);
        unsigned terminal = mapped[i] == PL_OK || mapped[i] == PL_TIMEOUT ||
                            mapped[i] == PL_IO || mapped[i] == PL_CANCELLED;
        if (terminal)
            CHECK(r == PL_OK && out.token == 1 &&
                  out.status == (unsigned)mapped[i] && !service.active);
        else
            CHECK(r == mapped[i] && out.token == 77 && service.active);
    }
    puts("PL_STATUS_PASS");
    return 0;
}
