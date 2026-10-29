#define main public_tests
#include "../../../labs/peripheral_depth/dma/tests.c"
#undef main

int main(void)
{
    const uint32_t bases[] = {0x10000000u, 0x20000000u, 0x2002fff0u,
                              0x30040040u, 0x30047ff0u, 0xfffffff0u};
    unsigned serial = 0;
    for (unsigned board = 0; board < 2; board++)
        for (unsigned cached = 0; cached < 2; cached++)
            for (unsigned b = 0; b < 6; b++)
                for (unsigned offset = 0; offset < 36; offset++)
                    for (unsigned len = 0; len < 68; len++)
                        for (unsigned width = 1; width < 5; width++)
                        {
                            uint32_t total = 777;
                            struct pd_extent e = {
                                bases[b] + offset,
                                board ? 0x30046000u : 0x20010000u,
                                len,
                                64,
                                64,
                                width};
                            int r = pd_validate(board, cached, &e, 1, &total);
                            printf("V %u %u %u %u %u %u %u %d %u\n", serial++,
                                   board, cached, b, offset, len, width, r,
                                   total);
                        }
    for (unsigned cancel = 0; cancel < 2; cancel++)
        for (unsigned en = 0; en < 2; en++)
            for (unsigned empty = 0; empty < 2; empty++)
                for (unsigned flags = 0; flags < 64; flags++)
                    for (unsigned remaining = 0; remaining <= 18; remaining++)
                    {
                        struct dg_model m = {0};
                        struct pd_transfer s = {0};
                        struct pd_io io = bus(&m);
                        struct pd_extent e = item(PD_F439, 16);
                        struct pd_result out = {99, 99, 99, 99};
                        int start = pd_submit(&s, io, PD_F439, 0, &e, 1, 1);
                        if (cancel)
                            (void)pd_cancel(&s, io, 1);
                        m.snap = (struct pd_snapshot){1, en, remaining, flags,
                                                      empty};
                        int r = pd_poll(&s, io, 1);
                        int f = pd_finalize(&s, io, 1, &out);
                        printf("S %u %u %u %u %u %d %d %d %u %u %u\n", cancel,
                               en, empty, flags, remaining, start, r, f,
                               out.status, out.complete_bytes,
                               out.possibly_written_bytes);
                    }
    for (unsigned width = 1; width <= 4; width *= 2)
        for (unsigned cut = 0; cut <= 49; cut++)
        {
            struct pd_extent v[3], out[4];
            unsigned n = 99;
            for (unsigned k = 0; k < 3; k++)
            {
                v[k] = item(PD_F439, 16);
                v[k].width = width;
                v[k].source += k * 32;
                v[k].destination += k * 32;
            }
            int r = pd_suffix(v, 3, cut, out, &n);
            printf("R %u %u %d %u", width, cut, r, n);
            if (r == PD_OK)
                for (unsigned k = 0; k < n; k++)
                    printf(" %u %u %u", out[k].source, out[k].destination,
                           out[k].bytes);
            puts("");
        }
    return 0;
}
