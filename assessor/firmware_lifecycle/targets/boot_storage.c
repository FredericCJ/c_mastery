#include "boot_storage.h"
#ifndef BS_MUTANT
#define BS_MUTANT 0
#endif
static uint32_t get(const unsigned char *p)
{
    return (uint32_t)p[0] | (uint32_t)p[1] << 8 | (uint32_t)p[2] << 16 |
           (uint32_t)p[3] << 24;
}

static void put(unsigned char *p, uint32_t n, unsigned count)
{
    for (unsigned b = 0; b < count; ++b)
    {
        p[b] = (unsigned char)(n >> (8 * b));
    }
}

int bs_read_image(eg_read_fn read, void *ctx, const struct fb_policy *policy,
                  struct fb_layout layout, unsigned core, struct fb_image *out)
{
    if (!read || !policy || !out || core > 1 || layout.base < 32 ||
        (layout.base & 511u) || layout.limit <= layout.base ||
        layout.limit - layout.base < 2112)
    {
        return 0;
    }
    unsigned char image[2080], signature[64];
    uint32_t word;
    for (unsigned i = 0; i < 8; ++i)
    {
        if (read(ctx, layout.base - 32 + 4 * i, &word) != 1)
        {
            return 0;
        }
        put(image + 4 * i, word, 4);
    }
    uint32_t extent = get(image + 20);
    if (extent < 514 || extent > 2048)
    {
        return 0;
    }
    for (uint32_t offset = 0; offset < extent; offset += 4)
    {
        if (read(ctx, layout.base + offset, &word) != 1)
        {
            return 0;
        }
        put(image + 32 + offset, word,
            extent - offset < 4 ? extent - offset : 4);
    }
    for (unsigned i = 0; i < 16; ++i)
    {
        if (read(ctx, layout.base + 2048 + 4 * i, &word) != 1)
        {
            return 0;
        }
        put(signature + 4 * i, word, 4);
    }
    return fb_admit(policy, layout, core, image, 32u + extent, signature, out);
}

int bs_select(eg_read_fn read, void *ctx, const struct fb_policy *p,
              const struct bs_layouts *choices, const uint32_t bases[2],
              struct bs_selection *out)
{
    if (!read || !p || !choices || !bases || !out || bases[0] == bases[1])
    {
        return 0;
    }
    unsigned char records[2][128];
    unsigned valid[2] = {0, 0};
    struct fb_image images[2][2];
    for (unsigned record = 0; record < 2; ++record)
    {
        if (!eg_snapshot(read, ctx, bases[record], records[record]) ||
            !fh_record_valid(records[record]))
        {
            continue;
        }
        if (BS_MUTANT == 2)
        {
            out->sequence = get(records[record] + 4);
        }
        unsigned good = 1;
        for (unsigned core = 0; core < 2; ++core)
        {
            uint32_t vector = get(records[record] + 80 + 4 * core);
            int which = -1;
            for (unsigned slot = 0; slot < 2; ++slot)
            {
                if (choices->core[core][slot].base == vector)
                {
                    if (which != -1)
                    {
                        good = 0;
                        break;
                    }
                    which = (int)slot;
                }
            }
            if (!good || which < 0 ||
                !bs_read_image(read, ctx, p,
                               choices->core[core][(unsigned)which], core,
                               &images[record][core]))
            {
                good = 0;
                break;
            }
        }
        valid[record] =
            good && (BS_MUTANT == 1 ||
                     fb_record_matches(records[record], images[record]));
    }
    int chosen;
    if (!valid[0])
    {
        chosen = valid[1] ? 1 : -1;
    }
    else if (!valid[1])
    {
        chosen = 0;
    }
    else
    {
        int selected = fm_recover(records[0], records[1]);
        if (selected < 0)
        {
            return -1;
        }
        chosen = selected - 1;
    }
    if (chosen < 0)
    {
        return 0;
    }
    out->sequence = get(records[chosen] + 4);
    out->record = (unsigned)chosen;
    out->image[0] = images[chosen][0];
    out->image[1] = images[chosen][1];
    return 1;
}
