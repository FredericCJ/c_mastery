#include "collection.h"
#include <stdio.h>
#include <string.h>
static struct vg_vg g;

int main(void)
{
    unsigned e[6], roots, expected, cases = 0;
    while (scanf("%u%u%u%u%u%u%u%u", e, e + 1, e + 2, e + 3, e + 4, e + 5,
                 &roots, &expected) == 8)
    {
        struct vg_ref r[3], scalar;
        uint16_t dead;
        unsigned n;
        memset(&g, 0, sizeof g);
        if (vg_reset(&g, 0) || vg_bind(&g, cvm_int(0), &scalar))
        {
            return 2;
        }
        for (unsigned i = 0; i < 3; i++)
        {
            if (vg_pair(&g, scalar, scalar, &r[i]))
            {
                return 3;
            }
        }
        for (unsigned i = 0; i < 6; i++)
        {
            if (e[i] > 3 ||
                vg_store(&g, r[i / 2], i % 2, e[i] ? r[e[i] - 1] : scalar))
            {
                return 4;
            }
        }
        /* Three actual root classes, not an adapter-only root bitmap. */
        if (roots & 1)
        {
            if (cvm_root_push(&g.vm, r[0].value))
            {
                return 5;
            }
        }
        if (roots & 2)
        {
            g.vm.stack[g.vm.sp++] = r[1].value;
        }
        if (roots & 4)
        {
            g.vm.depth = 1;
            g.vm.frames[0].locals[0] = r[2].value;
            g.vm.frames[0].initialized[0] = 1;
        }
        for (unsigned i = 0; i < 3; i++)
        {
            if (vg_weak_set(&g, i, r[i]))
            {
                return 6;
            }
        }
        if (vg_begin(&g))
        {
            return 7;
        }
        int s;
        unsigned steps = 0;
        do
        {
            s = vg_step(&g, 1, &n);
            if (n > 1 || ++steps > 4)
            {
                return 8;
            }
        } while (s == VG_OK);
        if (s != VG_DONE || vg_commit(&g, &dead) ||
            dead != (uint16_t)(7 ^ expected))
        {
            return 9;
        }
        for (unsigned i = 0; i < 3; i++)
        {
            if (g.vm.heap[i].used != ((expected >> i) & 1) ||
                g.weak_present[i] != ((expected >> i) & 1))
            {
                return 10;
            }
        }
        cases++;
    }
    if (!feof(stdin) || cases != 32768)
    {
        return 11;
    }
    printf("VG_ORACLE_PASS cases=%u root_classes=3 weak_slots=3\n", cases);
    return 0;
}
