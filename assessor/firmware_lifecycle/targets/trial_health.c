#include "trial_health.h"
#ifndef TH_MUTANT
#define TH_MUTANT 0
#endif
int th_health_accept(unsigned c, uint32_t g, uint32_t expected, uint32_t result)
{
    const uint32_t answer[2] = {671377293u, 3011442293u};
    return c < 2 && g && g == expected &&
           (TH_MUTANT == 3 || result == answer[c]);
}

enum fh_result th_commit_begin(struct fm_recycle *r, struct fm_io io,
                               const struct bs_selection *o,
                               const struct bs_selection *t,
                               const struct fw_peer *p, uint32_t parked)
{
    if (!r || !o || !t || !p || o->record > 1 || t->record != o->record ||
        !o->sequence || o->sequence == UINT32_MAX ||
        t->sequence != o->sequence + 1 || t->image[0].core != 0 ||
        t->image[1].core != 1 || !t->image[0].generation ||
        t->image[0].generation != t->image[1].generation ||
        t->image[0].generation <= o->image[0].generation ||
        t->image[1].generation <= o->image[1].generation ||
        (TH_MUTANT != 1 && parked != t->image[0].generation) ||
        (TH_MUTANT != 2 && p->generation != t->image[0].generation))
        return FH_REJECT;
    unsigned char hashes[64];
    uint32_t vectors[2], versions[2];
    for (unsigned c = 0; c < 2; ++c)
    {
        if (t->image[c].base == o->image[c].base ||
            t->image[c].version < o->image[c].version)
            return FH_REJECT;
        vectors[c] = t->image[c].base;
        versions[c] = t->image[c].version;
        for (unsigned i = 0; i < 32; ++i)
            hashes[32 * c + i] = t->image[c].digest[i];
    }
    return fm_recycle_begin(r, io, 0x080C0000u + o->record * 0x20000u, p,
                            t->sequence, hashes, vectors, versions);
}
