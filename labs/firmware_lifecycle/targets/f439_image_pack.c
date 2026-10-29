/* Host fixture only: RFC8032 public test seed. Never link this into a target. */
#include "recovery_transport.h"
#include "monocypher-ed25519.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void put(unsigned char *p, uint32_t v)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        p[i] = (unsigned char)(v >> (8 * i));
    }
}

int main(int argc, char **argv)
{
    if (argc != 7)
    {
        return 2;
    }
    unsigned core = (unsigned)strtoul(argv[1], 0, 0),
             slot = (unsigned)strtoul(argv[2], 0, 0),
             gen = (unsigned)strtoul(argv[3], 0, 0);
    if (core != 0 || slot > 1 || !gen)
    {
        return 2;
    }
    unsigned char image[2144], payload[2049],
        seed[32] = {0x9d, 0x61, 0xb1, 0x9d, 0xef, 0xfd, 0x5a, 0x60,
                    0xba, 0x84, 0x4a, 0xf4, 0x92, 0xec, 0x2c, 0xc4,
                    0x44, 0x49, 0xc5, 0x69, 0x7b, 0x32, 0x69, 0x19,
                    0x70, 0x3b, 0xac, 0x03, 0x1c, 0xae, 0x7f, 0x60},
        secret[64];
    FILE *f = fopen(argv[4], "rb");
    if (!f)
    {
        return 3;
    }
    size_t n = fread(payload, 1, sizeof payload, f);
    int err = ferror(f);
    if (fclose(f) || err || n > 2048 || n < 514)
    {
        return 3;
    }
    memset(image, 255, sizeof image);
    memset(image, 0, 32);
    uint32_t base = 0x08080200u + slot * 0x20000u;
    put(image, 0x31423646);
    image[4] = 1;
    image[5] = (unsigned char)core;
    put(image + 8, gen);
    put(image + 12, gen);
    put(image + 16, base);
    put(image + 20, (uint32_t)n);
    memcpy(image + 32, payload, n);
    struct fb_policy policy = {
        .present = 2, .floor = {1, 1}, .generation_floor = 1};
    crypto_ed25519_key_pair(secret, policy.keys[0], seed);
    crypto_ed25519_sign(image + 2080, secret, image, n + 32);
    struct fb_layout layout = {base, base + 0x1E00,
                               core ? 0x30000000 : 0x20000000,
                               core ? 0x30020000 : 0x20020000};
    struct fb_image admitted;
    if (!fb_admit(&policy, layout, core, image, n + 32, image + 2080,
                  &admitted))
    {
        return 4;
    }
    f = fopen(argv[5], "wb");
    if (!f)
    {
        return 3;
    }
    size_t w = fwrite(image, 1, sizeof image, f);
    if (fclose(f) || w != sizeof image)
    {
        return 3;
    }
    unsigned char frame[2156] = {0};
    memcpy(frame, "B6UP", 4);
    frame[4] = 1;
    frame[5] = (unsigned char)core;
    frame[6] = (unsigned char)slot;
    frame[8] = (unsigned char)(n + 32);
    frame[9] = (unsigned char)((n + 32) >> 8);
    memcpy(frame + 12, image, n + 32);
    memcpy(frame + 44 + n, image + 2080, 64);
    f = fopen(argv[6], "wb");
    if (!f)
    {
        return 3;
    }
    w = fwrite(frame, 1, n + 108, f);
    if (fclose(f) || w != n + 108)
    {
        return 3;
    }
    printf("F4_PACK_PASS:core%u slot%u gen%u payload%zu vector%08x entry%08x\n",
           core, slot, gen, n, admitted.base, admitted.entry);
    return 0;
}
