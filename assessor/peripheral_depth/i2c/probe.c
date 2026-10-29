#define main pi_public_test_main
#include "../../../labs/peripheral_depth/i2c/tests.c"
#undef main

static int advance(struct pi_i2c *s, struct dg_model *m, uint32_t t)
{
    switch (s->phase)
    {
        case 1:
        case 5:
            return event(s, m, PI_SB, t);
        case 2:
        case 6:
            m->receiver = s->phase == 6;
            return event(s, m, PI_ADDR, t);
        case 3:
            return event(s, m, PI_TXE, t);
        case 4:
        case 8:
            return event(s, m, PI_BTF | PI_RXNE, t);
        case 7:
            return event(s, m, PI_RXNE, t);
        case 9:
            m->r[PI_CR1] &= ~PI_STOP;
            m->r[PI_SR2] = 0;
            return event(s, m, 0, t);
        default:
            return PI_INVALID;
    }
}

int main(void)
{
    for (unsigned mhz = 1; mhz <= 45; mhz++)
    {
        struct pi_i2c s = {0};
        struct dg_model m = {0};
        struct pi_io io = {&m, rd, wr};
        struct pu_clock c = {mhz * 1000000u, 1, 1, 1, 1, 0};
        int rc = pi_open(&s, io, &c);
        printf("C %u %d %u %u %u\n", mhz, rc, m.r[PI_CCR], m.r[PI_TRISE],
               m.r[PI_CR2]);
    }
    for (unsigned addr = 8; addr < 120; addr++)
    {
        for (unsigned ci = 0; ci < 3; ci++)
        {
            for (unsigned count = 1; count <= 2; count++)
            {
                for (unsigned wrap = 0; wrap < 2; wrap++)
                {
                    struct pi_i2c s = {0};
                    struct dg_model m = {0};
                    struct pi_io io = {&m, rd, wr};
                    struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
                    uint8_t out[2] = {77, 88};
                    unsigned cmd = (unsigned[]){0, 16, 255}[ci];
                    uint32_t base = wrap ? UINT32_MAX - 4u : 0;
                    int rc = PI_BUSY;
                    m.count = count;
                    m.incoming[0] = (uint8_t)addr;
                    m.incoming[1] = (uint8_t)cmd;
                    if (pi_open(&s, io, &c) || pi_begin(&s, io, 1, 1, addr, cmd,
                                                        count, out, base, 100))
                    {
                        return 1;
                    }
                    for (unsigned t = 1; t <= 10 && rc == PI_BUSY; t++)
                    {
                        rc = advance(&s, &m, base + t);
                    }
                    printf("I %u %u %u %u %d %u %u %u %u %u %u %u %u\n", addr,
                           cmd, count, wrap, rc, m.tx[0], m.tx[1], m.tx[2],
                           s.received, m.stop, m.violation, out[0], out[1]);
                }
            }
        }
    }
    for (unsigned count = 1; count <= 2; count++)
    {
        for (unsigned stall = 1; stall <= 9; stall++)
        {
            for (unsigned wrap = 0; wrap < 2; wrap++)
            {
                if (stall == (count == 1 ? 8u : 7u))
                {
                    continue;
                }
                struct pi_i2c s = {0};
                struct dg_model m = {0};
                struct pi_io io = {&m, rd, wr};
                struct pu_clock c = {16000000, 1, 1, 1, 1, 0};
                uint8_t out[2] = {77, 88};
                uint32_t base = wrap ? UINT32_MAX - 4u : 0;
                unsigned t = 1;
                m.count = count;
                m.incoming[0] = 90;
                m.incoming[1] = 195;
                if (pi_open(&s, io, &c) ||
                    pi_begin(&s, io, 1, 1, 0x52, 0x10, count, out, base, 100))
                {
                    return 1;
                }
                while (s.phase != stall)
                {
                    if (t > 10 || advance(&s, &m, base + t++) != PI_BUSY)
                    {
                        return 1;
                    }
                }
                m.r[PI_SR1] = 0;
                if (stall == 9)
                {
                    m.r[PI_CR1] |= PI_STOP;
                }
                int rc = pi_step(&s, io, 1, base + 100);
                printf("T %u %u %u %d %u %u %u %u %u %u\n", count, stall, wrap,
                       rc, s.result, s.command_sent, s.received, m.violation,
                       out[0], out[1]);
            }
        }
    }
    for (unsigned need = 0; need <= 10; need++)
    {
        for (unsigned half = 1; half <= 4; half++)
        {
            for (unsigned stretch = 0; stretch <= 4; stretch++)
            {
                for (unsigned wrap = 0; wrap < 2; wrap++)
                {
                    struct pi_recovery s = {0};
                    struct dg_lines l = {{1, 1}, 1, need, 0, 0};
                    struct pi_lines io = {&l, line_read, line_release};
                    uint32_t base = wrap ? UINT32_MAX - 4u : 0;
                    int rc = PI_BUSY;
                    if (pi_recover_begin(&s, io, 1, 0, base, 300, half))
                    {
                        return 1;
                    }
                    for (unsigned t = 0; t <= 300 && rc == PI_BUSY; t++)
                    {
                        l.held_scl = stretch == 4 || t < stretch;
                        rc = pi_recover_step(&s, io, base + t);
                    }
                    printf("R %u %u %u %u %d %u %u %u\n", need, half, stretch,
                           wrap, rc, s.pulses, l.released[0], l.released[1]);
                }
            }
        }
    }
    for (unsigned value = 0; value <= 255; value++)
    {
        for (unsigned inc = 0; inc < 2; inc++)
        {
            for (unsigned loss = 0; loss < 2; loss++)
            {
                uint8_t d = (uint8_t)value, out = 177;
                int rc = pi_device(0x10 + inc, loss, &d, &out);
                printf("D %u %u %u %d %u %u\n", value, inc, loss, rc, d, out);
            }
        }
    }
    for (unsigned inc = 0; inc < 2; inc++)
    {
        for (unsigned applied = 0; applied < 2; applied++)
        {
            for (unsigned status = 0; status <= PI_IO; status++)
            {
                printf("P %u %u %u %d\n", inc, applied, status,
                       pi_retry_policy(0x10 + inc, applied, status));
            }
        }
    }
    return 0;
}
