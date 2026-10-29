#include "build_cache.h"
#include "arch.h"
#ifndef MPB_MUTATION
#define MPB_MUTATION 0
#endif
static int valid(const struct mpb_entry *e)
{
    return e && e->input_length <= 512 && e->artifact_length < 256;
}

static int equal(const unsigned char *a, const unsigned char *b, size_t n)
{
    for (size_t i = 0; i < n; ++i)
        if (a[i] != b[i])
            return 0;
    return 1;
}

int mpb_plan(const struct mpb_node *nodes, size_t count, unsigned forced,
             struct mpb_plan *output)
{
    struct mpb_plan p = {0};
    const struct mpb_node *sorted[8] = {0};
    struct mp_build_node base[8] = {0};
    unsigned char descriptions[8][64];
    size_t lengths[8];
    uint64_t keys[8];
    unsigned closure[8] = {0};
    size_t i, j, k;
    if (!output || count > 8 || (count && !nodes) || forced > 1)
        return MPB_INVALID;
    for (i = 0; i < count; ++i)
    {
        if (nodes[i].id >= count || sorted[nodes[i].id] ||
            (nodes[i].dependencies & ~((1u << (unsigned)count) - 1u)))
            return MPB_INVALID;
        sorted[nodes[i].id] = &nodes[i];
        for (j = 0; j < 6; ++j)
            if (nodes[i].inputs[j].length > 8)
                return MPB_INVALID;
    }
    for (i = 0; i < count; ++i)
    {
        const struct mpb_node *n = sorted[i];
        size_t used = 0;
        unsigned char wire[5];
        if (ar_encode(2, (struct ar_record){n->revision}, wire, 5) != AR_OK)
            return MPB_INVALID;
        descriptions[i][used++] = (unsigned char)i;
        descriptions[i][used++] = (unsigned char)n->dependencies;
        for (k = 0; k < 5; ++k)
            descriptions[i][used++] = wire[k];
        for (j = 0; j < 6; ++j)
        {
            size_t length = (MPB_MUTATION == 1 && j == MPB_ENVIRONMENT)
                                ? 0
                                : n->inputs[j].length;
            descriptions[i][used++] = (unsigned char)length;
            for (k = 0; k < length; ++k)
                descriptions[i][used++] = n->inputs[j].bytes[k];
        }
        lengths[i] = used;
        base[i] = (struct mp_build_node){n->dependencies, descriptions[i], used,
                                         0, 0};
        p.dependencies[i] = n->dependencies;
    }
    if (!mp_build(base, count, p.order, keys))
        return MPB_CYCLE;
    p.count = count;
    for (i = 0; i < count; ++i)
    {
        unsigned id = p.order[i];
        closure[id] = 1u << id;
        for (j = 0; j < count; ++j)
            if (p.dependencies[id] & (1u << (unsigned)j))
                closure[id] |= closure[j];
    }
    for (i = 0; i < count; ++i)
    {
        struct mpb_entry *e = &p.entries[i];
        size_t used = 0;
        e->key = forced ? 0 : keys[i];
        e->input[used++] = 1;
        e->input[used++] = (unsigned char)i;
        e->input[used++] = (unsigned char)closure[i];
        for (j = 0; j < count; ++j)
            if (closure[i] & (1u << (unsigned)j))
            {
                e->input[used++] = (unsigned char)lengths[j];
                for (k = 0; k < lengths[j]; ++k)
                    e->input[used++] = descriptions[j][k];
            }
        e->input_length = used;
        if (!mp_emit_table(&keys[i], 1, e->artifact, sizeof e->artifact,
                           &e->artifact_length))
            return MPB_FULL;
    }
    *output = p;
    return MPB_OK;
}

int mpb_match(const struct mpb_entry *expected, const struct mpb_entry *cached)
{
    if (!valid(expected) || !valid(cached))
        return 0;
    if (expected->key != cached->key)
        return 0;
    if (MPB_MUTATION == 2)
        return 1;
    return expected->input_length == cached->input_length &&
           equal(expected->input, cached->input, expected->input_length) &&
           (MPB_MUTATION == 5 ||
            (expected->artifact_length == cached->artifact_length &&
             equal((const unsigned char *)expected->artifact,
                   (const unsigned char *)cached->artifact,
                   expected->artifact_length)));
}

int mpb_encode(const struct mpb_entry *e, unsigned id, unsigned char *bytes,
               size_t capacity, size_t *used)
{
    unsigned char staged[800];
    size_t n = 0, i;
    if (!valid(e) || id > 7 || !used || (capacity && !bytes))
        return MPB_INVALID;
    staged[n++] = 'M';
    staged[n++] = 'P';
    staged[n++] = 'B';
    staged[n++] = 1;
    staged[n++] = (unsigned char)id;
    for (i = 0; i < 8; ++i)
        staged[n++] = (unsigned char)(e->key >> (8 * i));
    staged[n++] = (unsigned char)(e->input_length >> 8);
    staged[n++] = (unsigned char)e->input_length;
    for (i = 0; i < e->input_length; ++i)
        staged[n++] = e->input[i];
    staged[n++] = (unsigned char)(e->artifact_length >> 8);
    staged[n++] = (unsigned char)e->artifact_length;
    for (i = 0; i < e->artifact_length; ++i)
        staged[n++] = (unsigned char)e->artifact[i];
    if (n > capacity)
        return MPB_FULL;
    for (i = 0; i < n; ++i)
        bytes[i] = staged[i];
    *used = n;
    return MPB_OK;
}

int mpb_begin(struct mpb_schedule *s, const struct mpb_plan *p)
{
    if (!s || !p || p->count > 8)
        return MPB_INVALID;
    if (s->running || s->pending)
        return MPB_BUSY;
    if (s->epoch == UINT64_MAX)
        return MPB_EXHAUSTED;
    *s = (struct mpb_schedule){
        p, s->epoch + 1, (1u << (unsigned)p->count) - 1u, 0, 0, 0};
    return MPB_OK;
}

int mpb_claim(struct mpb_schedule *s, struct mpb_ticket *ticket)
{
    if (!s || !ticket || !s->plan || !s->epoch)
        return MPB_INVALID;
    if (s->failed)
        return MPB_FAILED;
    for (unsigned i = 0; i < s->plan->count; ++i)
        if ((s->pending & (1u << i)) &&
            (MPB_MUTATION == 3 || !(s->plan->dependencies[i] & ~s->done)))
        {
            s->pending &= ~(1u << i);
            s->running |= 1u << i;
            *ticket = (struct mpb_ticket){s, s->epoch, i};
            return MPB_OK;
        }
    return s->running || s->pending ? MPB_BUSY : MPB_DONE;
}

int mpb_finish(struct mpb_schedule *s, struct mpb_ticket t,
               const struct mpb_entry *produced)
{
    if (!s || !s->plan)
        return MPB_INVALID;
    if (t.owner != s || t.epoch != s->epoch || t.id >= s->plan->count ||
        !(s->running & (1u << t.id)))
        return MPB_STALE;
    if (MPB_MUTATION != 4)
        s->running &= ~(1u << t.id);
    if (!produced || !mpb_match(&s->plan->entries[t.id], produced))
    {
        s->failed = 1;
        s->pending = 0;
        return MPB_FAILED;
    }
    s->done |= 1u << t.id;
    return MPB_OK;
}

int mpb_publish(const struct mpb_schedule *s, struct mpb_plan *out)
{
    if (!s || !out || !s->plan)
        return MPB_INVALID;
    if (MPB_MUTATION != 6 && (s->running || s->pending))
        return MPB_BUSY;
    if (s->failed)
        return MPB_FAILED;
    *out = *s->plan;
    return MPB_OK;
}
