/* Adapter composition against the existing EnModel/EpModel. Selected address
 * routing is observable; this is not an Ethernet device emulator. */
#include "board.h"
#include "ethernet_followthrough/phy/model.h"
#include "ethernet_followthrough/ring/model.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                        \
    do                                  \
    {                                   \
        if (!(x))                       \
        {                               \
            puts("NB_BOARD_CHECK " #x); \
            return 1;                   \
        }                               \
    } while (0)
extern struct en_memory nb_memory;
static struct nb_board board;
static struct en_model ring_model;
static struct ep_model phy_model;
static uint32_t gpio[7][10], rcc[18], pmc, demcr, ctrl, count, last_write;
static unsigned violations, frozen;

uint32_t en_port_load(uint32_t a)
{
    if (a == 0xe000edfc)
    {
        return demcr;
    }
    if (a == 0xe0001000)
    {
        return ctrl;
    }
    if (a == 0xe0001004)
    {
        if (!frozen && (ctrl & 1) && (demcr & 0x1000000))
        {
            count += 8;
        }
        return count;
    }
    if (a >= 0x40028000u && a <= 0x4002901cu)
    {
        uint32_t v = 0;
        struct en_bus b = en_model_bus(&ring_model);
        if (b.read(b.context, a - 0x40028000u, &v))
        {
            violations++;
        }
        return v;
    }
    if (a >= 0x40023800u && a <= 0x40023844u)
    {
        return rcc[(a - 0x40023800u) / 4];
    }
    if (a == 0x40013804)
    {
        return pmc;
    }
    if (a >= 0x40020000u && a < 0x40021c00u)
    {
        unsigned p = (a - 0x40020000u) / 0x400u, o = (a - 0x40020000u) % 0x400u;
        if (o <= 0x24)
        {
            return gpio[p][o / 4];
        }
    }
    violations++;
    return 0;
}

void en_port_store(uint32_t a, uint32_t v)
{
    if (a == 0xe000edfc)
    {
        demcr = v;
        return;
    }
    if (a == 0xe0001000)
    {
        ctrl = v;
        return;
    }
    if (a >= 0x40028000u && a <= 0x4002901cu)
    {
        if ((uint32_t)(count - last_write) < 720)
        {
            violations++;
        }
        last_write = count;
        struct en_bus b = en_model_bus(&ring_model);
        if (b.settle(b.context) || b.write(b.context, a - 0x40028000u, v))
        {
            violations++;
        }
        return;
    }
    if (a >= 0x40023800u && a <= 0x40023844u)
    {
        rcc[(a - 0x40023800u) / 4] = v;
        return;
    }
    if (a == 0x40013804)
    {
        if (!(rcc[0x10 / 4] & 0x2000000u) || (rcc[0x30 / 4] & 0xe000000u))
        {
            violations++;
        }
        pmc = v;
        return;
    }
    if (a >= 0x40020000u && a < 0x40021c00u)
    {
        unsigned p = (a - 0x40020000u) / 0x400u, o = (a - 0x40020000u) % 0x400u;
        if (o <= 0x24)
        {
            if (!(rcc[0x30 / 4] & (1u << p)))
            {
                violations++;
            }
            gpio[p][o / 4] = v;
            return;
        }
    }
    violations++;
}

void en_port_barrier(void)
{
    struct en_bus b = en_model_bus(&ring_model);
    b.barrier(b.context);
}

int ep_test_read(uint32_t a, uint32_t *v)
{
    if (a != 0x40028010 && a != 0x40028014)
    {
        return 1;
    }
    return ep_model_read(&phy_model, (a - 0x40028010) / 4, v);
}

int ep_test_write(uint32_t a, uint32_t v)
{
    if (a != 0x40028010 && a != 0x40028014)
    {
        return 1;
    }
    return ep_model_write(&phy_model, (a - 0x40028010) / 4, v);
}

static void setup(void)
{
    memset(&board, 0, sizeof board);
    memset(gpio, 0, sizeof gpio);
    memset(rcc, 0, sizeof rcc);
    pmc = demcr = ctrl = count = last_write = violations = frozen = 0;
    en_model_init(&ring_model, &nb_memory, 0x20001000);
    ep_model_setup(&phy_model, 0);
}

static int begin(void)
{
    struct na_config c = {{2, 0, 0, 0, 0, 2}, {192, 0, 2, 2}, {192, 0, 2, 1}};
    return nb_board_begin(&board, &nb_memory, 0x20001000,
                          (struct ep_mmio_clock){&phy_model, ep_model_clock}, c,
                          1, 1000, 16);
}

int nb_board_tests(void)
{
    setup();
    CHECK(begin() == NB_WAIT);
    int e = NB_WAIT;
    for (unsigned i = 0; i < 512 && e == NB_WAIT; i++)
    {
        e = nb_board_step(&board);
    }
    CHECK(e == NB_OK && board.phase == NB_BOARD_READY &&
          board.net.tcp.state == NC_LISTEN);
    CHECK(!violations && !ring_model.violation && !phy_model.violation);
    CHECK(pmc == 0x800000u && (rcc[0x30 / 4] & 0xe000047u) == 0xe000047u &&
          !(rcc[0x10 / 4] & 0x2000000u));
    CHECK(gpio[0][0] == 0x8028u && gpio[1][0] == 0x08000000u &&
          gpio[2][0] == 0xa08u && gpio[6][0] == 0x08800000u);
    CHECK(ring_model.reg[0x100c / 4] == 0x20001000 &&
          ring_model.reg[0x1010 / 4] == 0x20001040);
    phy_model.phy[0][1] = 0x20;
    CHECK(nb_step(&board.net, phy_model.tick + 1) == NB_WAIT &&
          board.net.phase == NB_STOPPING);
    CHECK(nb_stop_step(&board.net, phy_model.tick + 2) == NB_STOPPED);
    CHECK(en_target_close(&board.target) == ED_OK);
    setup();
    phy_model.phy[0][31] = 0x1014;
    CHECK(begin() == NB_WAIT);
    e = NB_WAIT;
    for (unsigned i = 0; i < 512 && e == NB_WAIT; i++)
    {
        e = nb_board_step(&board);
    }
    CHECK(e == NB_OK && board.phy.mode.speed == 10 &&
          !(ring_model.reg[0] & 0x4000u));
    CHECK(nb_stop(&board.net, phy_model.tick + 1) == NB_WAIT);
    CHECK(nb_stop_step(&board.net, phy_model.tick + 2) == NB_STOPPED &&
          en_target_close(&board.target) == ED_OK);
    setup();
    phy_model.stuck = 1;
    CHECK(begin() == NB_FAULT && board.phase == NB_BOARD_FAILED);
    setup();
    CHECK(begin() == NB_WAIT);
    phy_model.tick += 500;
    CHECK(nb_board_step(&board) == NB_FAULT);
    setup();
    CHECK(begin() == NB_WAIT);
    ring_model.reset_stuck = 1;
    e = NB_WAIT;
    for (unsigned i = 0; i < 512 && e == NB_WAIT; i++)
    {
        e = nb_board_step(&board);
    }
    CHECK(e == NB_FAULT && board.phase == NB_BOARD_FAILED &&
          board.ring.reset_pending);
    setup();
    CHECK(begin() == NB_WAIT);
    frozen = 1;
    e = NB_WAIT;
    for (unsigned i = 0; i < 512 && e == NB_WAIT; i++)
    {
        e = nb_board_step(&board);
    }
    CHECK(e == NB_FAULT && board.phase == NB_BOARD_FAILED &&
          board.target.opened);
    puts("NB_BOARD_PASS actual_F439_ports_PHY_ring_boundaries_model_only");
    return 0;
}
