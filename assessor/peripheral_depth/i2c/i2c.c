#include "i2c.h"
#ifndef PI_MUTANT
#define PI_MUTANT 0
#endif
static int valid(struct pi_io io)
{
    return io.read && io.write;
}

static int due(uint32_t now, uint32_t then)
{
    return (uint32_t)(now - then) < UINT32_C(0x80000000);
}

static unsigned pending(uint32_t cr)
{
    return (cr & (PI_START | PI_STOP | PI_PEC)) != 0;
}

static int finish(struct pi_i2c *s)
{
    int r = (int)s->result;
    if (r == PI_BUSY)
    {
        for (unsigned i = 0; i < s->count; i++)
            s->output[i] = s->partial[i];
        r = PI_OK;
    }
    s->phase = 0;
    s->master = 0;
    s->result = (unsigned)r;
    return r;
}

int pi_open(struct pi_i2c *s, struct pi_io io, const struct pu_clock *c)
{
    struct pu_rates rates;
    if (!s || !valid(io) || !c || pu_rates(c, &rates) != PU_OK ||
        rates.apb1 < 2000000u || rates.apb1 > 42000000u ||
        rates.apb1 % 1000000u)
        return PI_INVALID;
    if (s->phase)
        return PI_BUSY;
    /* Read SR2 before SR1; no address transaction is pending at open. */
    if ((io.read(io.context, PI_SR2) & 3u) ||
        pending(io.read(io.context, PI_CR1)))
        return PI_BUSY;
    io.write(io.context, PI_CR1, 0);
    io.write(io.context, PI_CR2, rates.apb1 / 1000000u);
    io.write(io.context, PI_OAR1, 1u << 14);
    io.write(io.context, PI_OAR2, 0);
    io.write(io.context, PI_CCR, (rates.apb1 + 159999u) / 160000u);
    io.write(io.context, PI_TRISE, rates.apb1 / 1000000u + 1u);
    io.write(io.context, PI_FLTR, 0);
    io.write(io.context, PI_CR1, PI_PE);
    s->ready = 1;
    s->clock = rates.apb1;
    s->generation = c->generation;
    return PI_OK;
}

int pi_begin(struct pi_i2c *s, struct pi_io io, uint32_t generation,
             uint32_t token, unsigned address, unsigned command, unsigned count,
             uint8_t *out, uint32_t now, unsigned budget)
{
    if (!s || !valid(io) || !out || !token || address < 8 || address > 119 ||
        command > 255 || count < 1 || count > 2 || !budget ||
        budget >= UINT32_C(0x80000000))
        return PI_INVALID;
    if (!s->ready || generation != s->generation || token <= s->last_token)
        return PI_STALE;
    if (s->phase || (io.read(io.context, PI_SR2) & 2u) ||
        pending(io.read(io.context, PI_CR1)))
        return PI_BUSY;
    s->token = s->last_token = token;
    s->last = now;
    s->deadline = now + budget;
    s->address = address;
    s->command = command;
    s->count = count;
    s->output = out;
    s->phase = 1;
    s->received = s->command_sent = s->master = s->stop_requested = 0;
    s->result = PI_BUSY;
    s->partial[0] = s->partial[1] = 0;
    io.write(io.context, PI_CR1, PI_PE | PI_ACK | PI_START);
    return PI_OK;
}

int pi_cancel(struct pi_i2c *s, struct pi_io io, uint32_t token)
{
    if (!s || !valid(io))
        return PI_INVALID;
    if (!s->phase || token != s->token)
        return PI_STALE;
    if (s->result == PI_BUSY)
        s->result = PI_TIMEOUT;
    s->phase = 10;
    return PI_BUSY;
}

int pi_step(struct pi_i2c *s, struct pi_io io, uint32_t token, uint32_t now)
{
    uint32_t sr, cr;
    if (!s || !valid(io))
        return PI_INVALID;
    if (!s->phase || token != s->token)
        return PI_STALE;
    if ((uint32_t)(now - s->last) >= UINT32_C(0x80000000))
        return PI_INVALID;
    s->last = now;
    sr = io.read(io.context, PI_SR1);
    cr = io.read(io.context, PI_CR1);
    /* ARLO releases the bus in hardware. Never send STOP into the winner. */
    if (sr & PI_ARLO)
    {
        io.write(io.context, PI_SR1, 0xdf00u & ~(PI_ARLO | PI_AF | PI_BERR));
        if (PI_MUTANT == 3)
            io.write(io.context, PI_CR1, PI_PE | PI_STOP);
        s->master = 0;
        s->result = PI_ARBITRATION;
        return finish(s);
    }
    /* ES0206 §2.10.1: spurious master BERR, clear and continue. */
    if (sr & PI_BERR)
    {
        io.write(io.context, PI_SR1, 0xdf00u & ~PI_BERR);
        if (s->berr != UINT32_MAX)
            ++s->berr;
    }
    if (sr & PI_AF)
    {
        io.write(io.context, PI_SR1, 0xdf00u & ~PI_AF);
        s->result =
            (s->phase == 2 || s->phase == 6) ? PI_ADDRESS_NACK : PI_DATA_NACK;
        s->phase = 10;
    }
    if (sr & (PI_OVR | 0xd000u))
    {
        io.write(io.context, PI_SR1, 0xdf00u & ~(sr & (PI_OVR | 0xd000u)));
        s->result = PI_IO;
        s->phase = 10;
    }
    switch (s->phase)
    {
        case 1:
            if (sr & PI_SB)
            {
                s->master = 1;
                io.write(io.context, PI_DR, s->address << 1);
                s->phase = 2;
            }
            break;
        case 2:
            if (sr & PI_ADDR)
            {
                (void)io.read(io.context, PI_SR2);
                s->phase = 3;
            }
            break;
        case 3:
            if (sr & PI_TXE)
            {
                io.write(io.context, PI_DR, s->command);
                s->command_sent = 1;
                s->phase = 4;
            }
            break;
        case 4:
            if ((sr & PI_BTF) && !pending(cr))
            {
                io.write(io.context, PI_CR1, (cr & ~PI_POS) | PI_START);
                s->phase = 5;
            }
            break;
        case 5:
            if (sr & PI_SB)
            {
                io.write(io.context, PI_DR, (s->address << 1) | 1u);
                s->phase = 6;
            }
            break;
        case 6:
            if ((sr & PI_ADDR) && !pending(cr))
            {
                unsigned config =
                    (cr & ~(PI_ACK | PI_POS)) | (s->count == 2 ? PI_POS : 0);
                if (PI_MUTANT == 1)
                    (void)io.read(io.context, PI_SR2);
                io.write(io.context, PI_CR1, config);
                if (PI_MUTANT != 1)
                    (void)io.read(io.context, PI_SR2);
                if (s->count == 1)
                {
                    io.write(io.context, PI_CR1, config | PI_STOP);
                    s->stop_requested = 1;
                    s->phase = 7;
                }
                else
                    s->phase = 8;
            }
            break;
        case 7:
            if (sr & PI_RXNE)
            {
                s->partial[0] = (uint8_t)io.read(io.context, PI_DR);
                s->received = 1;
                s->phase = 9;
            }
            break;
        case 8:
            if ((sr & (PI_MUTANT == 2 ? PI_RXNE : PI_BTF)) && !pending(cr))
            {
                io.write(io.context, PI_CR1, cr | PI_STOP);
                s->stop_requested = 1;
                s->partial[0] = (uint8_t)io.read(io.context, PI_DR);
                s->partial[1] = (uint8_t)io.read(io.context, PI_DR);
                s->received = 2;
                s->phase = 9;
            }
            break;
        case 9:
            if (!(cr & PI_STOP))
            {
                /* STOP has completed; own address0/general-call disabled cannot match. */
                if (!(io.read(io.context, PI_SR2) & 1u))
                    return finish(s);
            }
            break;
        case 10:
            /* START may complete after timeout before the normal SB branch runs. */
            if (sr & PI_SB)
                s->master = 1;
            /* Do not write CR1 while a prior command is still pending. */
            if (pending(cr) && PI_MUTANT != 6)
                break;
            if (sr & PI_ADDR)
            {
                io.write(io.context, PI_CR1, PI_PE);
                (void)io.read(io.context, PI_SR2);
            }
            if (!s->master || s->stop_requested)
                return finish(s);
            io.write(io.context, PI_CR1, PI_PE | PI_STOP);
            s->stop_requested = 1;
            break;
        default:
            return PI_INVALID;
    }
    if (s->result == PI_BUSY && due(now, s->deadline))
    {
        s->result = PI_TIMEOUT;
        s->phase = 10;
    }
    return PI_BUSY;
}

int pi_recover_begin(struct pi_recovery *s, struct pi_lines io,
                     unsigned exclusive, unsigned peers, uint32_t now,
                     unsigned budget, unsigned half)
{
    if (!s || !io.read || !io.release || exclusive > 1 || peers > 1 || !half ||
        half >= UINT32_C(0x40000000) || budget <= half ||
        budget >= UINT32_C(0x80000000))
        return PI_INVALID;
    if ((!exclusive || peers) && PI_MUTANT != 4)
        return PI_FORBIDDEN;
    if (s->active)
        return PI_BUSY;
    s->active = 1;
    s->phase = 1;
    s->pulses = 0;
    s->result = PI_BUSY;
    s->last = now;
    s->deadline = now + budget;
    s->half = half;
    s->due = now;
    io.release(io.context, PI_SDA, 1);
    io.release(io.context, PI_SCL, 1);
    return PI_OK;
}

static int recover_end(struct pi_recovery *s, struct pi_lines io,
                       unsigned result)
{
    io.release(io.context, PI_SDA, 1);
    io.release(io.context, PI_SCL, 1);
    s->active = 0;
    s->result = result;
    return (int)result;
}

int pi_recover_step(struct pi_recovery *s, struct pi_lines io, uint32_t now)
{
    if (!s || !io.read || !io.release)
        return PI_INVALID;
    if (!s->active)
        return PI_STALE;
    if ((uint32_t)(now - s->last) >= UINT32_C(0x80000000))
        return PI_INVALID;
    s->last = now;
    if (due(now, s->deadline))
        return recover_end(s, io, PI_TIMEOUT);
    switch (s->phase)
    {
        case 1: /* Released high may still be held low by another device. */
            if (!io.read(io.context, PI_SCL))
                break;
            s->due = now + s->half;
            s->phase = 2;
            break;
        case 2:
            if (!due(now, s->due))
                break;
            if (!io.read(io.context, PI_SCL))
            {
                s->phase = 1;
                break;
            }
            if (io.read(io.context, PI_SDA))
            {
                io.release(io.context, PI_SCL, 0);
                io.release(io.context, PI_SDA, 0);
                s->due = now + s->half;
                s->phase = 4;
            }
            else if (s->pulses == 9)
                return recover_end(s, io, PI_IO);
            else
            {
                io.release(io.context, PI_SCL, 0);
                s->due = now + s->half;
                s->phase = 3;
            }
            break;
        case 3:
            if (due(now, s->due))
            {
                io.release(io.context, PI_SCL, 1);
                ++s->pulses;
                s->phase = 1;
            }
            break;
        case 4:
            if (due(now, s->due))
            {
                io.release(io.context, PI_SCL, 1);
                s->phase = 5;
            }
            break;
        case 5:
            if (io.read(io.context, PI_SCL))
            {
                s->due = now + s->half;
                s->phase = 6;
            }
            break;
        case 6:
            if (!io.read(io.context, PI_SCL))
            {
                s->phase = 5;
                break;
            }
            if (due(now, s->due))
            {
                io.release(io.context, PI_SDA, 1);
                s->due = now + s->half;
                s->phase = 7;
            }
            break;
        case 7:
            if (!io.read(io.context, PI_SCL))
                return recover_end(s, io, PI_IO);
            if (due(now, s->due))
                return recover_end(s, io,
                                   io.read(io.context, PI_SDA) ? PI_OK : PI_IO);
            break;
        default:
            return PI_INVALID;
    }
    return PI_BUSY;
}

int pi_device(unsigned command, unsigned lose, uint8_t *device, uint8_t *reply)
{
    if (!device || !reply || lose > 1 || (command != 0x10 && command != 0x11))
        return PI_INVALID;
    if (command == 0x11)
    {
        if (*device == 255)
            return PI_IO;
        ++*device;
    }
    if (lose)
        return PI_TIMEOUT;
    *reply = *device;
    return PI_OK;
}

int pi_retry_policy(unsigned command, unsigned applied, unsigned status)
{
    if ((command != 0x10 && command != 0x11) || applied > 1 ||
        (status != PI_OK && status != PI_TIMEOUT && status != PI_ADDRESS_NACK &&
         status != PI_DATA_NACK && status != PI_ARBITRATION && status != PI_IO))
        return PI_INVALID;
    if (status == PI_OK)
        return PI_OK;
    if (command == 0x11 && applied && PI_MUTANT != 5)
        return PI_UNCERTAIN;
    return PI_BUSY; /* Eligible to retry after separate bus admission/backoff. */
}
