#include "adc.h"
#ifndef PA_MUTANT
#define PA_MUTANT 0
#endif
static int io_valid(struct pa_io io)
{
    return io.read && io.write;
}

static int time_valid(struct pa_adc *s, uint32_t now)
{
    return now - s->last < UINT32_C(0x80000000);
}

int pa_timing(const struct pu_clock *clock, const struct pa_config *cfg,
              struct pa_timing *out)
{
    static const unsigned samples[8] = {3, 15, 28, 56, 84, 112, 144, 480};
    struct pu_rates rates;
    struct pa_timing t;
    if (!clock || !cfg || !out || cfg->sample > 7 || cfg->channels < 1 ||
        cfg->channels > 2 || cfg->timer > 1 || pu_rates(clock, &rates) != 0)
        return PA_INVALID;
    unsigned div;
    for (div = 2; div <= 8; div += 2)
        if (rates.apb2 % div == 0 && rates.apb2 / div >= 600000u &&
            rates.apb2 / div <= 16000000u)
            break;
    if (div > 8 || rates.apb2 % div || rates.apb2 / div < 600000u)
        return PA_INVALID;
    t.hz = rates.apb2 / div;
    t.prescaler = div / 2 - 1;
    t.total_cycles =
        (samples[cfg->sample] + (PA_MUTANT == 1 ? 0 : 12)) * cfg->channels;
    t.conversion_us = (t.total_cycles * 1000000u + t.hz - 1) / t.hz;
    *out = t;
    return PA_OK;
}

int pa_open(struct pa_adc *s, struct pa_io io, const struct pu_clock *clock,
            const struct pa_config *cfg, uint32_t now)
{
    struct pa_timing timing;
    if (!s || !io_valid(io) || pa_timing(clock, cfg, &timing) != PA_OK)
        return PA_INVALID;
    if (s->phase)
        return PA_BUSY;
    /* Exclusive ADC1/common-clock ownership; ADC2/3 disabled. Stop before editing
       sequence registers: ES0206 2.5.1 invalidates a mid-conversion restart
       claim. */
    io.write(io.context, PA_CR2, 0);
    io.write(io.context, PA_CCR, timing.prescaler << 16);
    io.write(io.context, PA_CR1, cfg->channels == 2 ? PA_SCAN : 0);
    io.write(io.context, PA_SMPR1, 0);
    io.write(io.context, PA_SMPR2, cfg->sample | (cfg->sample << 3));
    io.write(io.context, PA_SQR1, (cfg->channels - 1) << 20);
    io.write(io.context, PA_SQR2, 0);
    io.write(io.context, PA_SQR3, cfg->channels == 2 ? 32 : 0);
    io.write(io.context, PA_SR, 0);
    io.write(io.context, PA_CR2, PA_EOCS | PA_ADON);
    s->generation = clock->generation;
    s->last = now;
    s->due = 0;
    s->warm_sampled = 0;
    s->phase = 1;
    s->channels = cfg->channels;
    s->timer = cfg->timer;
    s->received = 0;
    s->output = 0;
    return PA_OK;
}

int pa_ready(struct pa_adc *s, uint32_t now)
{
    if (!s || !time_valid(s, now))
        return PA_INVALID;
    if (s->phase != 1)
        return s->phase == 2 ? PA_OK : PA_BUSY;
    if (!s->warm_sampled)
    {
        s->warm_sampled = 1;
        s->last = now;
        s->due = now + 4;
        return PA_BUSY;
    }
    s->last = now;
    if (PA_MUTANT != 2 && now - s->due >= UINT32_C(0x80000000))
        return PA_BUSY;
    s->phase = 2;
    return PA_OK;
}

int pa_begin(struct pa_adc *s, struct pa_io io, uint32_t generation,
             uint32_t token, uint16_t *out, uint32_t now, uint32_t budget)
{
    if (!s || !io_valid(io) || !out || !token || !budget ||
        budget >= UINT32_C(0x80000000) || !time_valid(s, now))
        return PA_INVALID;
    if (generation != s->generation || token <= s->last_token)
        return PA_STALE;
    if (s->phase != 2)
        return PA_BUSY;
    io.write(io.context, PA_SR, 0);
    s->token = s->last_token = token;
    s->output = out;
    s->last = now;
    s->deadline = now + budget;
    s->received = 0;
    s->phase = 3;
    io.write(io.context, PA_CR2,
             PA_ADON | PA_EOCS | (s->timer ? PA_EXTERNAL : PA_SWSTART));
    return PA_OK;
}

static int finish(struct pa_adc *s, struct pa_io io, int result)
{
    /* ADON=0 stops conversion; software trigger source must be stopped by its
       owner before another open. No DMA borrower exists in this PIO successor. */
    io.write(io.context, PA_CR2, 0);
    io.write(io.context, PA_SR, 0);
    if (result == PA_OK)
        for (unsigned i = 0; i < s->channels; i++)
            s->output[i] = s->partial[i];
    s->phase = 0;
    s->output = 0;
    return result;
}

int pa_step(struct pa_adc *s, struct pa_io io, uint32_t token, uint32_t now)
{
    if (!s || !io_valid(io) || !time_valid(s, now))
        return PA_INVALID;
    if (s->phase != 3 || token != s->token)
        return PA_STALE;
    s->last = now;
    unsigned status = io.read(io.context, PA_SR);
    if (PA_MUTANT != 3 && (status & PA_OVR))
        return finish(s, io, PA_OVERRUN);
    if (now - s->deadline < UINT32_C(0x80000000))
        return finish(s, io, PA_TIMEOUT);
    if (status & PA_EOC)
    {
        unsigned value = io.read(io.context, PA_DR) & 4095u;
        /* An overrun between first status and DR makes channel identity uncertain.
         */
        if (PA_MUTANT != 4 && PA_MUTANT != 3 &&
            (io.read(io.context, PA_SR) & PA_OVR))
            return finish(s, io, PA_OVERRUN);
        s->partial[s->received++] = (uint16_t)value;
        if (PA_MUTANT == 5)
            s->output[s->received - 1] = (uint16_t)value;
        if (s->received == s->channels)
            return finish(s, io, PA_OK);
    }
    return PA_BUSY;
}

int pa_cancel(struct pa_adc *s, struct pa_io io, uint32_t token)
{
    if (!s || !io_valid(io))
        return PA_INVALID;
    if (s->phase != 3 || token != s->token)
        return PA_STALE;
    return finish(s, io, PA_TIMEOUT);
}

int pa_calibrate(unsigned code, unsigned c0, unsigned c1, unsigned v0,
                 unsigned v1, int32_t *out)
{
    if (!out || code > 4095 || c0 >= c1 || c1 > 4095 || v0 >= v1 || v1 > 3600)
        return PA_INVALID;
    int32_t delta = (int32_t)code - (int32_t)c0;
    int32_t numerator = delta * (int32_t)(v1 - v0), denom = (int32_t)(c1 - c0);
    /* Nearest integer millivolt, half away from zero. Bounds keep signed products
       within 14,742,000; extrapolation is explicit and can be outside supply. */
    if (PA_MUTANT != 6)
        numerator += numerator < 0 ? -(denom / 2) : denom / 2;
    *out = (int32_t)v0 + numerator / denom;
    return PA_OK;
}

int pa_ramp(unsigned index, unsigned steps, unsigned low, unsigned high,
            unsigned *out)
{
    if (!out || steps < 2 || steps > 4096 || index >= steps || low >= high ||
        high > 4095)
        return PA_INVALID;
    *out = low + (index * (high - low)) / (steps - 1);
    return PA_OK;
}

int pa_threshold(unsigned *state, int32_t input, int32_t low, int32_t high)
{
    if (!state || *state > 1 || low >= high)
        return PA_INVALID;
    if (PA_MUTANT == 7)
        *state = input >= high;
    else if (input >= high)
        *state = 1;
    else if (input <= low)
        *state = 0;
    return PA_OK;
}
