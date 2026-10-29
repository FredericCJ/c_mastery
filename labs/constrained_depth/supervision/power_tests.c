#include "power.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                     \
    do                                               \
    {                                                \
        if (!(x))                                    \
        {                                            \
            printf("CS_POWER_CHECK:%d\n", __LINE__); \
            return 1;                                \
        }                                            \
    } while (0)

struct cr_model
{
    uint32_t r[CS_REG_COUNT];
    unsigned writes, idles;
    enum cs_reg reg[64];
    uint32_t value[64];
};

static uint32_t read_reg(void *c, enum cs_reg r)
{
    return ((struct cr_model *)c)->r[r];
}

static void write_reg(void *c, enum cs_reg r, uint32_t v)
{
    struct cr_model *m = c;
    m->reg[m->writes] = r;
    m->value[m->writes++] = v;
    m->r[r] = v;
}

static void idle(void *c)
{
    struct cr_model *m = c;
    ++m->idles;
    m->r[CS_RCC_CR] = 0;
}

static void init(struct cr_model *m)
{
    *m = (struct cr_model){0};
    m->r[CS_RCC_CR] = 3;
    m->r[CS_RCC_APB1ENR] = 1u << 28;
    m->r[CS_PWR_CR] = 0xc000;
}

int main(void)
{
    struct cr_model m;
    init(&m);
    struct cs_bus b = {&m, read_reg, write_reg, idle};
    struct cs_stop s;
    CHECK(cs_stop_init(&s) == CS_OK);
    for (unsigned missing = 0; missing < 5; ++missing)
        CHECK(cs_stop_enter(&s, &b, 31u ^ (1u << missing)) == CS_INVALID &&
              m.writes == 0 && s.phase == 0);
    m.r[CS_IWDG_SR] = 3;
    CHECK(cs_stop_enter(&s, &b, 31) == CS_BUSY && m.writes == 0);
    m.r[CS_IWDG_SR] = 0;

    const enum cs_reg regs[] = {CS_RCC_CR, CS_RCC_CFGR, CS_PWR_CR,
                                CS_SCR,    CS_DBG,      CS_SYSTICK};

    const uint32_t wrong[] = {0x1000003, 8, 0x10000, 4, 2, 1};
    for (unsigned i = 0; i < 6; ++i)
    {
        uint32_t saved = m.r[regs[i]];
        m.r[regs[i]] = wrong[i];
        CHECK(cs_stop_enter(&s, &b, 31) == CS_INVALID && m.writes == 0);
        m.r[regs[i]] = saved;
    }
    CHECK(cs_stop_enter(&s, &b, 31) == CS_OK && s.phase == 1 && m.idles == 1 &&
          m.writes == 4 && m.r[CS_SCR] == 0 && m.r[CS_PWR_CR] == 0xc000);
    CHECK(m.reg[0] == CS_PWR_CR && m.reg[1] == CS_SCR && m.value[1] == 4 &&
          m.reg[2] == CS_SCR && m.value[2] == 0);
    struct cs_stop before = s;
    CHECK(cs_stop_finish(&s, &b, 1, 100, 120) == CS_WAIT &&
          !memcmp(&before, &s, sizeof s));
    m.r[CS_RCC_CR] = 3;
    CHECK(cs_stop_finish(&s, &b, 1, 120, 100) == CS_INVALID &&
          !memcmp(&before, &s, sizeof s));
    CHECK(cs_stop_finish(&s, &b, 1, 100, 120) == CS_OK && s.epoch == 1 &&
          s.elapsed_known && s.elapsed == 20);
    CHECK(cs_stop_finish(&s, &b, 1, 100, 120) == CS_INVALID && s.epoch == 1);
    CHECK(cs_stop_init(&s) == CS_OK);
    init(&m);
    CHECK(cs_stop_enter(&s, &b, 31) == CS_OK);
    m.r[CS_RCC_CR] = 3;
    CHECK(cs_stop_finish(&s, &b, 0, 100, 1) == CS_OK && !s.elapsed_known &&
          !s.elapsed && s.epoch == 1);
    CHECK(cs_stop_init(&s) == CS_OK);
    s.epoch = UINT32_MAX;
    init(&m);
    CHECK(cs_stop_enter(&s, &b, 31) == CS_EXHAUSTED && !m.writes);
    init(&m);
    CHECK(cs_iwdg_start(&b, 7, 99, 1) == CS_INVALID && !m.writes);
    m.r[CS_IWDG_SR] = 3;
    CHECK(cs_iwdg_start(&b, 6, 4095, 3) == CS_WAIT && m.writes == 4 &&
          m.value[0] == 0xcccc && m.value[1] == 0x5555);
    init(&m);
    CHECK(cs_iwdg_start(&b, 6, 4095, 3) == CS_OK && m.writes == 5 &&
          m.value[4] == 0xaaaa);
    struct cs_world w;
    CHECK(cs_init(&w, 2, 1) == CS_OK);
    m.writes = 0;
    CHECK(cs_iwdg_feed(&w, &b) == CS_WAIT && !m.writes);
    CHECK(cs_progress(&w, 0) == CS_OK && cs_iwdg_feed(&w, &b) == CS_WAIT &&
          !m.writes);
    CHECK(cs_progress(&w, 1) == CS_OK && cs_iwdg_feed(&w, &b) == CS_OK &&
          m.writes == 1 && m.value[0] == 0xaaaa);
    puts(
        "CS_POWER_PASS:Stop=HSI16,quiescent clock=required timestamp=known-or-unknown epoch=once IWDG=all-progress,PVU-RVU refusal=start-not-rolled-back");
    return 0;
}
