/* Reuse the same original register/cut model, not another mocked algorithm. */
#define main maintenance_checks
#include "flash_maintenance_test.c"
#undef main
#include "boot_pair.h"
#include "monocypher-ed25519.h"

static void p32(unsigned char *p, uint32_t n)
{
    for (unsigned i = 0; i < 4; ++i)
    {
        p[i] = (unsigned char)(n >> (8 * i));
    }
}

int main(void)
{
    C(maintenance_checks() == 0);
    struct fb_policy policy = {0};
    unsigned char secret[64], seed[32] = {0};
    crypto_ed25519_key_pair(secret, policy.keys[0], seed);
    policy.present = 2;
    policy.generation_floor = 5;
    policy.floor[0] = policy.floor[1] = 3;
    struct fb_image pair[2][2];
    for (unsigned generation = 0; generation < 2; ++generation)
    {
        for (unsigned core = 0; core < 2; ++core)
        {
            unsigned char image[608] = {0}, signature[64];
            uint32_t base =
                0x08040000u + core * 0x100000u + generation * 0x40000u;
            p32(image, 0x31423646u);
            image[4] = 1;
            image[5] = (unsigned char)core;
            p32(image + 8, 3 + generation);
            p32(image + 12, 5 + generation);
            p32(image + 16, base);
            p32(image + 20, 576);
            p32(image + 32, 0x20010000u);
            p32(image + 36, base + 513);
            crypto_ed25519_sign(signature, secret, image, 608);
            C(fb_admit(&policy,
                       (struct fb_layout){base, base + 0x20000u, 0x20000000u,
                                          0x20020000u},
                       core, image, 608, signature, &pair[generation][core]));
        }
    }
    struct fw_peer old_peer = {.generation = 5, .health = 3, .confirmed = 1},
                   new_peer = {.generation = 6};
    unsigned char old_record[128], new_record[128];
    C(fb_joint_record(pair[0], &old_peer, 9, old_record));
    C(fw_peer_step(&new_peer, 0, 6, FW_HEALTHY) == H7_OK);
    C(fw_peer_step(&new_peer, 1, 6, FW_FAIL_PEER) == H7_OK);
    init(1);
    store(0x080C0000u, old_record);
    C(!fb_joint_record(pair[1], &new_peer, 10, new_record) &&
      !m.writes); /* Old generation remains published. */
    new_peer = (struct fw_peer){.generation = 6, .health = 3, .confirmed = 1};
    C(fb_joint_record(pair[1], &new_peer, 10, new_record));
    unsigned char hashes[64];
    uint32_t vectors[2], versions[2];
    for (unsigned c = 0; c < 2; ++c)
    {
        memcpy(hashes + 32 * c, pair[1][c].digest, 32);
        vectors[c] = pair[1][c].base;
        versions[c] = pair[1][c].version;
    }
    unsigned cuts[40] = {1, 2, 2048, 4096};
    unsigned nc = 4;
    for (unsigned i = 1; i <= 32; ++i)
    {
        cuts[nc++] = 4096 + i;
    }
    cuts[nc++] = 4129;
    cuts[nc++] = 6176;
    cuts[nc++] = 8224;
    cuts[nc++] = 0;
    for (unsigned cut_index = 0; cut_index < nc; ++cut_index)
    {
        init(1);
        store(0x080C0000u, old_record);
        m.cut = cuts[cut_index];
        struct fm_recycle transaction = {0};
        C(fm_recycle_begin(&transaction, io(), 0x080C0000u, &new_peer, 10,
                           hashes, vectors, versions) == FH_PENDING);
        enum fh_result result = FH_PENDING;
        for (unsigned i = 0; i < 2200 && result == FH_PENDING && !m.dead; ++i)
        {
            finish();
            if (!m.dead)
            {
                result = fm_recycle_poll(&transaction);
            }
        }
        C(m.dead || result == FH_DONE);
        unsigned char a[128], b[128];
        bytes(0x080C0000u, a);
        bytes(0x080E0000u, b);
        int chosen = fm_recover(a, b);
        C(chosen == 1 || chosen == 2);
        C(fb_record_matches(chosen == 1 ? a : b, pair[chosen - 1]));
        C(!m.violations);
    }
    puts(
        "FB_COMPOSITION_PASS:two authenticated image pairs,one peer fails,40 register-cut recoveries bind exact pair");
    return 0;
}
