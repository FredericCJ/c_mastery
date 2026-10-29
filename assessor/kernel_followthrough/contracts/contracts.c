#include "contracts.h"

static uint32_t u32(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static uint64_t u64(const unsigned char *p)
{
    return (uint64_t)u32(p) | (uint64_t)u32(p + 4) << 32;
}

int kf_boot(const unsigned char *p, size_t n, struct kf_boot *out)
{
    struct kf_boot b = {0};
    if (!p || !out || cmk_boot_parse(p, n, &b.core) != 0)
    {
        return 0;
    }
    size_t total = u32(p), off = 8;
    unsigned command = 0;
    while (off < total)
    {
        /* The retained parser validated every extent first; keep these checks
         * explicit so this extension never relies on unbounded pointer walks. */
        if (total - off < 8)
        {
            return 0;
        }
        uint32_t type = u32(p + off), size = u32(p + off + 4);
        if (size < 8 || size > total - off)
        {
            return 0;
        }
        ++b.visited;
        if (type == 1)
        {
            if (command++ || size < 9)
            {
                return 0;
            }
            size_t len = 0;
            while (len < size - 8 && p[off + 8 + len])
            {
                ++len;
            }
            if (len == size - 8)
            {
                return 0;
            }
            b.command = p + off + 8;
            b.command_length = len;
        }
        size_t padding = (8u - (size & 7u)) & 7u;
        if (padding > total - off - size)
        {
            return 0;
        }
        size_t step = (size_t)size + padding;
        off += step;
        b.next = off;
    }
    if (off != total)
    {
        return 0;
    }
    *out = b;
    return 1;
}

int kf_frames(cmk_frames *f, const cmk_boot *b, const struct kf_span *r,
              size_t nr)
{
    if (!f || !b || (!r && nr) || b->entry_size < 24 ||
        b->map_bytes % b->entry_size)
    {
        return 0;
    }
    for (size_t i = 0; i < nr; ++i)
    {
        if (r[i].start > r[i].end)
        {
            return 0;
        }
    }
    for (size_t off = 0; off < b->map_bytes; off += b->entry_size)
    {
        uint64_t base = u64(b->map + off), length = u64(b->map + off + 8);
        if (length > UINT64_MAX - base)
        {
            return 0;
        }
    }
    /* No output mutation until all interval arithmetic is known valid. */
    for (unsigned page = 0; page < CMK_MAX_FRAMES; ++page)
    {
        uint64_t lo = (uint64_t)page * 4096, hi = lo + 4096;
        unsigned usable = 0, reserved = page == 0;
        for (size_t off = 0; off < b->map_bytes; off += b->entry_size)
        {
            uint64_t base = u64(b->map + off),
                     end = base + u64(b->map + off + 8);
            uint32_t type = u32(b->map + off + 16);
            if (type == 1 && base <= lo && hi <= end)
            {
                usable = 1;
            }
            if (type != 1 && base < hi && lo < end)
            {
                reserved = 1;
            }
        }
        for (size_t i = 0; i < nr; ++i)
        {
            if (r[i].start < r[i].end && r[i].start < hi && lo < r[i].end)
            {
                reserved = 1;
            }
        }
        f->state[page] = (unsigned char)(usable && !reserved);
    }
    return 1;
}

size_t kf_free(const struct kf_pool *p)
{
    size_t n = 0;
    for (unsigned i = 0; i < CMK_MAX_FRAMES; ++i)
    {
        n += p->frames.state[i] == 1;
    }
    return n;
}

int kf_allocate(struct kf_pool *p, struct kf_space *s, unsigned first,
                unsigned count, unsigned budget)
{
    if (!p || !s || !s->live || !count || count > 16 || first >= 512 ||
        count > 512 - first)
    {
        return 0;
    }
    for (unsigned i = 0; i < count; ++i)
    {
        if (s->pages[first + i].frame)
        {
            return 0;
        }
    }
    unsigned done = 0;
    for (; done < count; ++done)
    {
        uint32_t frame = done < budget ? cmk_frame_take(&p->frames) : 0;
        if (!frame)
        {
            break;
        }
        if (frame < 0x300000 || frame >= 0x400000)
        {
            (void)cmk_frame_put(&p->frames, frame);
            break;
        }
        unsigned char *bytes = p->bytes[(frame - 0x300000) / 4096];
        for (unsigned j = 0; j < 4096; ++j)
        {
            bytes[j] = 0;
        }
        s->pages[first + done] = (struct kf_pte){
            frame, 7}; /* present,user,writable; model NX by lack of execute */
    }
    if (done == count)
    {
        return 1;
    }
    for (unsigned i = 0; i < done; ++i)
    {
        (void)cmk_frame_put(&p->frames, s->pages[first + i].frame);
        s->pages[first + i] = (struct kf_pte){0};
    }
    return 0;
}

int kf_reap_user(struct kf_pool *p, struct kf_space *s, unsigned slot,
                 const struct kf_cpu *c)
{
    if (!p || !s || !c || s->live || slot == c->current ||
        slot == c->cr3_owner || slot == c->tss_owner)
    {
        return 0;
    }
    /* Old execution stack may still be active. Only user frames are reclaimed;
     * static kernel stacks and page tables are not frame-pool allocations. */
    for (unsigned i = 0; i < 512; ++i)
    {
        if (s->pages[i].frame)
        {
            if (cmk_frame_put(&p->frames, s->pages[i].frame) != 0)
            {
                return 0;
            }
            s->pages[i] = (struct kf_pte){0};
        }
    }
    return 1;
}

int kf_retire_slot(struct kf_space *s, unsigned slot, const struct kf_cpu *c)
{
    if (!s || !c || s->live || slot == c->current || slot == c->cr3_owner ||
        slot == c->tss_owner || slot == c->stack_owner)
    {
        return 0;
    }
    for (unsigned i = 0; i < 512; ++i)
    {
        if (s->pages[i].frame)
        {
            return 0;
        }
    }
    if (s->generation == UINT32_MAX)
    {
        s->retired = 1;
    }
    else
    {
        ++s->generation;
    }
    return 1;
}

int kf_permission(const uint64_t entry[4], unsigned write, unsigned execute,
                  unsigned user, unsigned *fault)
{
    if (!entry || !fault || write > 1 || execute > 1 || user > 1 ||
        write + execute > 1)
    {
        return 0;
    }
    unsigned present = 1, writable = 1, users = 1, nx = 0;
    for (unsigned i = 0; i < 4; ++i)
    {
        present &= (unsigned)(entry[i] & 1);
        writable &= (unsigned)((entry[i] >> 1) & 1);
        users &= (unsigned)((entry[i] >> 2) & 1);
        nx |= (unsigned)(entry[i] >> 63);
    }
    unsigned allowed = present && (!write || writable) && (!user || users) &&
                       (!execute || !nx);
    *fault = allowed
                 ? 0
                 : (present ? 1u : 0u) | 2u * write | 4u * user | 16u * execute;
    return (int)allowed;
}

int kf_next(const unsigned *alive, unsigned count, unsigned current)
{
    if (!alive || !count || count > 6 || current >= count)
    {
        return -1;
    }
    for (unsigned step = 1; step <= count; ++step)
    {
        if (alive[(current + step) % count])
        {
            return (int)((current + step) % count);
        }
    }
    return -1;
}

int kf_weighted(const unsigned *alive, const unsigned *weight, unsigned n,
                int64_t *credit)
{
    if (!alive || !weight || !credit || !n || n > 6)
    {
        return -1;
    }
    int64_t total = 0, next[6] = {0};
    int selected = -1;
    for (unsigned i = 0; i < n; ++i)
    {
        if (weight[i] > 65535)
        {
            return -1;
        }
        if (alive[i])
        {
            total += weight[i];
        }
    }
    if (!total)
    {
        return -1;
    }
    for (unsigned i = 0; i < n; ++i)
    {
        if (credit[i] < -393210 || credit[i] > 393210)
        {
            return -1;
        }
        if (alive[i] && weight[i])
        {
            next[i] = credit[i] + weight[i];
            if (selected < 0 || next[i] > next[(unsigned)selected])
            {
                selected = (int)i;
            }
        }
    }
    next[(unsigned)selected] -= total;
    for (unsigned i = 0; i < n; ++i)
    {
        credit[i] = next[i];
    }
    return selected;
}

int kf_load_image(struct kf_pool *pool, struct kf_space *space,
                  const unsigned char *p, size_t n, unsigned budget)
{
    if (!pool || !space || !space->live)
    {
        return 0;
    }
    struct cmk_elf elf;
    if (cmk_elf_parse(p, n, &elf))
    {
        return 0;
    }
    for (unsigned s = 0; s < elf.count; ++s)
    {
        const struct cmk_segment *seg = &elf.segments[s];
        if (s && elf.segments[s - 1].address > seg->address)
        {
            return 0;
        }
        unsigned first = (unsigned)((seg->address - CMK_USER_BASE) / 4096),
                 last = (unsigned)((seg->address + seg->memsz - 1 -
                                    CMK_USER_BASE) /
                                   4096);
        for (unsigned i = first; i <= last; ++i)
        {
            if (space->pages[i].frame)
            {
                return 0;
            }
        }
    }
    struct kf_space before = *space;
    cmk_frames state = pool->frames;
    unsigned used = 0;
    for (unsigned s = 0; s < elf.count; ++s)
    {
        const struct cmk_segment *seg = &elf.segments[s];
        unsigned first = (unsigned)((seg->address - CMK_USER_BASE) / 4096),
                 last = (unsigned)((seg->address + seg->memsz - 1 -
                                    CMK_USER_BASE) /
                                   4096);
        for (unsigned i = first; i <= last; ++i)
        {
            if (!kf_allocate(pool, space, i, 1, used < budget))
            {
                *space = before;
                pool->frames = state;
                return 0;
            }
            ++used;
            space->pages[i].permissions = 5u | ((seg->flags & 2) ? 2u : 0u) |
                                          ((seg->flags & 1) ? 8u : 0u);
        }
        for (uint64_t i = 0; i < seg->filesz; ++i)
        {
            uint64_t address = seg->address + i;
            unsigned page = (unsigned)((address - CMK_USER_BASE) / 4096),
                     index = (space->pages[page].frame - 0x300000) / 4096;
            pool->bytes[index][address % 4096] = p[seg->offset + i];
        }
    }
    return 1;
}

int kf_archive_typed(const unsigned char *p, size_t n, const char *name,
                     unsigned expected, const unsigned char **out,
                     size_t *length)
{
    if (!p || !name || !out || !length || n < 16 || p[0] != 'C' ||
        p[1] != 'M' || p[2] != 'K' || p[3] != 'A' || u32(p + 4) != 2 ||
        u32(p + 12) != n)
    {
        return 0;
    }
    unsigned count = u32(p + 8);
    if (!count || count > 16 || 16 + 40u * count > n)
    {
        return 0;
    }
    size_t name_len = 0;
    while (name_len < 24 && name[name_len])
    {
        ++name_len;
    }
    if (!name_len || name_len == 24 || expected < 1 || expected > 2)
    {
        return 0;
    }
    const unsigned char *found = 0;
    size_t found_len = 0;
    for (unsigned i = 0; i < count; ++i)
    {
        const unsigned char *r = p + 16 + 40 * i;
        unsigned len = 0;
        while (len < 24 && r[len])
        {
            ++len;
        }
        if (!len || len == 24)
        {
            return 0;
        }
        for (unsigned j = len; j < 24; ++j)
        {
            if (r[j])
            {
                return 0;
            }
        }
        uint32_t start = u32(r + 24), size = u32(r + 28), type = u32(r + 32);
        if (start < 16 + 40u * count || start > n || !size ||
            size > n - start || type < 1 || type > 2 || u32(r + 36))
        {
            return 0;
        }
        for (unsigned j = 0; j < i; ++j)
        {
            const unsigned char *old = p + 16 + 40 * j;
            unsigned same = 1;
            for (unsigned k = 0; k < 24; ++k)
            {
                if (r[k] != old[k])
                {
                    same = 0;
                }
            }
            if (same)
            {
                return 0;
            }
            uint32_t a = u32(old + 24), b = u32(old + 28);
            if (start < a + b && a < start + size)
            {
                return 0;
            }
        }
        unsigned same = len == name_len;
        for (unsigned j = 0; j < len && same; ++j)
        {
            if (r[j] != (unsigned char)name[j])
            {
                same = 0;
            }
        }
        if (same)
        {
            if (type != expected)
            {
                return 0;
            }
            found = p + start;
            found_len = size;
        }
    }
    if (!found)
    {
        return 0;
    }
    *out = found;
    *length = found_len;
    return 1;
}

uint32_t kf_handle_open(struct kf_handles *h, unsigned slot, unsigned resource,
                        unsigned rights)
{
    if (!h || slot >= 16 || !resource || !rights || (rights & ~3u) ||
        h->slot[slot].open || h->slot[slot].retired)
    {
        return 0;
    }
    struct kf_handle_slot *s = &h->slot[slot];
    if (!s->generation)
    {
        s->generation = 1;
    }
    s->open = 1;
    s->resource = resource;
    s->rights = rights;
    return cmk_handle(slot, s->generation);
}

int kf_handle_get(const struct kf_handles *h, uint64_t token, unsigned rights,
                  unsigned *resource)
{
    if (!h || !resource || token > UINT32_MAX || !rights || (rights & ~3u))
    {
        return 0;
    }
    unsigned encoded = (unsigned)(token & 65535);
    if (!encoded || encoded > 16)
    {
        return 0;
    }
    const struct kf_handle_slot *s = &h->slot[encoded - 1];
    if (!cmk_handle_valid((uint32_t)token, encoded - 1, s->generation,
                          (int)s->open) ||
        (s->rights & rights) != rights)
    {
        return 0;
    }
    *resource = s->resource;
    return 1;
}

int kf_handle_close(struct kf_handles *h, uint64_t token)
{
    unsigned resource;
    if (!kf_handle_get(h, token, 1, &resource) &&
        !kf_handle_get(h, token, 2, &resource))
    {
        return 0;
    }
    struct kf_handle_slot *s = &h->slot[(unsigned)(token & 65535) - 1];
    s->open = 0;
    s->rights = 0;
    s->resource = 0;
    if (s->generation == UINT16_MAX)
    {
        s->retired = 1;
    }
    else
    {
        ++s->generation;
    }
    return 1;
}

uint32_t kf_delegate(const struct kf_handles *source, uint64_t token,
                     struct kf_handles *dest, unsigned slot, unsigned rights)
{
    unsigned resource;
    if (!kf_handle_get(source, token, rights, &resource))
    {
        return 0;
    }
    return kf_handle_open(dest, slot, resource, rights);
}

int kf_line(struct kf_line *s, unsigned char c, char out[64], unsigned *length)
{
    if (!s || !out || !length)
    {
        return 0;
    }
    if (s->suppress_lf)
    {
        s->suppress_lf = 0;
        if (c == '\n')
        {
            return 0;
        }
    }
    if (c == '\r' || c == '\n')
    {
        s->suppress_lf = c == '\r';
        unsigned discarded = s->discard;
        s->discard = 0;
        if (discarded)
        {
            s->used = 0;
            return 2;
        }
        for (unsigned i = 0; i < s->used; ++i)
        {
            out[i] = s->line[i];
        }
        out[s->used] = 0;
        *length = s->used;
        s->used = 0;
        return 1;
    }
    if (s->discard)
    {
        return 0;
    }
    if (s->used == 63)
    {
        s->discard = 1;
        s->used = 0;
        return 0;
    }
    s->line[s->used++] = (char)c;
    return 0;
}
