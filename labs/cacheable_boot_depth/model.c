#include "model.h"

uint32_t cb_model_read(struct cb_model *m, uint32_t a)
{
    for (uint32_t i = 0; i < m->count; ++i)
        if (m->addresses[i] == a)
            return m->values[i];
    return 0;
}

void cb_model_set(struct cb_model *m, uint32_t a, uint32_t v)
{
    for (uint32_t i = 0; i < m->count; ++i)
        if (m->addresses[i] == a)
        {
            m->values[i] = v;
            return;
        }
    if (m->count == 96)
    {
        m->bad = 1;
        return;
    }
    m->addresses[m->count] = a;
    m->values[m->count++] = v;
}

static uint32_t read_word(void *c, uint32_t a)
{
    struct cb_model *m = c;
    if (a >= CB_LINE && a < CB_LINE + 32u)
        return m->dirty ? m->cache[(a - CB_LINE) / 4u]
                        : m->memory[(a - CB_LINE) / 4u];
    return cb_model_read(m, a);
}

static void write_word(void *c, uint32_t a, uint32_t v)
{
    struct cb_model *m = c;
    ++m->writes;
    if (a == 0xE000ED98u)
        m->selected = v;
    if (a == 0xE000EDA0u)
    {
        if (m->selected < 16)
            m->attrs[m->selected] = v;
        else
            m->bad = 1;
    }
    if (a == 0xE000ED9Cu)
    {
        if (m->selected < 16)
            m->bases[m->selected] = v;
        else
            m->bad = 1;
    }
    if (a == 0xE000EF60u)
    {
        unsigned set = (v >> 5) & 32767u, way = v >> 30;
        if (set >= m->sets || way >= 4u || m->invalid[set][way])
            m->bad = 1;
        else
            m->invalid[set][way] = 1;
        ++m->dc_writes;
    }
    if (a == 0xE000ED14u && (v & 0x10000u) && m->dc_writes != m->sets * 4u)
        m->enabled_before_invalidation = 1;
    if (a >= CB_LINE && a < CB_LINE + 32u)
    {
        ++m->line_writes;
        if (cb_model_read(m, 0xE000ED14u) & 0x10000u)
        {
            m->cache[(a - CB_LINE) / 4u] = v;
            m->dirty = 1;
        }
        else
            m->memory[(a - CB_LINE) / 4u] = v;
        return;
    }
    if (a == 0xE000EF68u)
    {
        if (v != CB_LINE)
            m->bad = 1;
        for (unsigned i = 0; i < 8; ++i)
            m->memory[i] = m->cache[i];
        m->dirty = 0;
        ++m->clean_writes;
    }
    if (a == 0x580244A0u && (v & 8u))
    {
        m->release_generation = m->memory[0];
        ++m->release_writes;
        if (m->dirty)
            m->bad = 1;
    }
    cb_model_set(m, a, v);
}

static void sync_word(void *c)
{
    struct cb_model *m = c;
    ++m->barriers;
}

static void instruction_sync(void *c)
{
    struct cb_model *m = c;
    ++m->instructions;
}

struct cb_io cb_model_io(struct cb_model *m)
{
    struct cb_io x;
    x.registers.ctx = m;
    x.registers.read = read_word;
    x.registers.write = write_word;
    x.registers.sync = sync_word;
    x.isb = instruction_sync;
    return x;
}

void cb_model_init(struct cb_model *m, unsigned sets)
{
    m->count = 0;
    m->selected = 0;
    m->sets = sets;
    m->writes = 0;
    m->line_writes = 0;
    m->dc_writes = 0;
    m->enabled_before_invalidation = 0;
    m->dirty = 0;
    m->release_generation = 0;
    m->release_writes = 0;
    m->clean_writes = 0;
    m->barriers = 0;
    m->instructions = 0;
    m->bad = 0;
    for (unsigned i = 0; i < 16; ++i)
    {
        m->attrs[i] = 0xFFFFFFFFu;
        m->bases[i] = 0;
    }
    for (unsigned i = 0; i < 8; ++i)
    {
        m->memory[i] = 0xA5A5A5A5u;
        m->cache[i] = 0xDDDDDDDDu;
    }
    for (unsigned s = 0; s < 512; ++s)
        for (unsigned w = 0; w < 4; ++w)
            m->invalid[s][w] = 0;
    cb_model_set(m, 0xE000ED90u, 8u << 8);
    cb_model_set(m, 0xE000ED80u, 0xF0000019u | ((sets - 1u) << 13));
    cb_model_set(m, 0x58024400u, 5);
    cb_model_set(m, 0x58000700u, 1);
    cb_model_set(m, 0x52002040u, 0x800);
    cb_model_set(m, 0x52002048u, 0x810);
}
