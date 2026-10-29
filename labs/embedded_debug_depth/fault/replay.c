#include "fault.h"
#include <stdio.h>

int main(void)
{
    const unsigned char image[32] = {1};
    uint32_t w[] = {1, 0x20000000, 2, 3, 12, 0x101, 0x100, 0x1000000};
    for (unsigned k = 0; k < 1024; ++k)
    {
        uint32_t status = ((k & 1) ? 0x200u : 0) | ((k & 2) ? 0x8000u : 0) |
                          ((k & 4) ? 0x400u : 0) | ((k & 8) ? 2u : 0) |
                          ((k & 16) ? 0x80u : 0) | ((k & 32) ? 0x1000u : 0);
        struct df_input in = {.cfsr = status,
                              .hfsr = (k & 64) ? 0x40000000 : 0,
                              .bfar = 0xdeadbeef,
                              .mmfar = 0xcafebabe,
                              .exc_return = (k & 128) ? 0xffffffed : 0xfffffffd,
                              .stack_address = 0x20001000,
                              .words = w,
                              .available = (k & 256) ? 7 : 8,
                              .readable = (k & 512) ? 0 : 1,
                              .aligned = 1,
                              .image = image};
        struct df_capture c;
        struct df_record r;
        struct df_flags f;
        unsigned secondary;
        df_init(&c);
        if (df_capture(&c, &in, 0, 0) || df_read(&c, &r, &secondary) ||
            df_flags(&r, &f))
        {
            return 1;
        }
        printf("%u %u %u %u %u %u %u %u %u\n", k, r.valid, r.bfar, r.mmfar,
               f.precise_bus, f.address_valid, f.forced, f.imprecise,
               secondary);
    }
    for (unsigned k = 0; k < 2048; ++k)
    {
        struct df_live_frame f[] = {
            {(k & 15) * 16, 0, 0, 0},
            {((k >> 4) & 15) * 16, 1, (k >> 8) & 1, (k >> 9) & 1}};
        struct df_budget b = {77, 77};
        int status = df_budget(f, 2, 256, (k >> 10) & 1, 0, &b);
        printf("B %u %d %u %u\n", k, status, b.required, b.fits);
    }
    return 0;
}
