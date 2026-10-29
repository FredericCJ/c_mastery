#include "image_transfer.h"
#include <stdio.h>
#include <string.h>
#define C(x)                                                           \
    do                                                                 \
    {                                                                  \
        if (!(x))                                                      \
        {                                                              \
            fprintf(stderr, "B06_IMAGE_ASSERT:%d:%s\n", __LINE__, #x); \
            return 1;                                                  \
        }                                                              \
    } while (0)

struct f4_storage
{
    struct fi_rotation durable;
    unsigned boundary, calls;
};

/* This separate store model retains the old committed policy until boundary6.
 * Each of six fixed fields is staged; only the final durable commit publishes. */
static int persist(void *ctx, const struct fi_rotation *s)
{
    struct f4_storage *p = ctx;
    p->calls++;
    if (p->boundary < 6)
        return 0;
    p->durable = *s;
    return 1;
}

int main(void)
{
    unsigned char target[32], old[32], record[24], sentinel[24], oldrec[24];
    for (unsigned i = 0; i < 32; i++)
    {
        target[i] = (unsigned char)(i * 7);
        old[i] = (unsigned char)(i * 3);
    }
    unsigned cuts = 0;
    for (unsigned unit = 4; unit <= 8; unit += 4)
    {
        struct fi_image state;
        C(fi_init(&state, unit));
        for (unsigned i = 0; i < 32 / unit; i++)
            C(fi_program(&state, old, i, unit * 8, 1));
        C(fi_commit(&state, old, 1, oldrec));
        C(fi_record_matches(old, oldrec));
        for (unsigned next = 0; next < 32 / unit; next++)
            for (unsigned bits = 0; bits <= unit * 8; bits++)
            {
                C(fi_init(&state, unit));
                for (unsigned i = 0; i < next; i++)
                    C(fi_program(&state, target, i, unit * 8, 1));
                C(fi_program(&state, target, next, bits, 0));
                memset(record, 0xAA, 24);
                memcpy(sentinel, record, 24);
                C(!fi_commit(&state, target, 2, record) &&
                  !memcmp(record, sentinel, 24));
                C(fi_record_matches(old, oldrec));
                cuts++;
            }
        C(fi_init(&state, unit));
        for (unsigned i = 0; i < 32 / unit; i++)
            C(fi_program(&state, target, i, unit * 8, 1));
        C(fi_commit(&state, target, 2, record) &&
          fi_record_matches(target, record));
        target[17] ^= 1;
        C(!fi_record_matches(target, record));
        target[17] ^= 1;
        C(fi_program(&state, target, 0, unit * 8, 0));
        memset(record, 0xAA, 24);
        memcpy(sentinel, record, 24);
        C(!fi_commit(&state, target, 2, record) &&
          !memcmp(record, sentinel, 24));
        C(fi_program(&state, target, 0, unit * 8, 1));
        struct fi_image saved = state;
        C(!fi_program(&state, old, 0, unit * 8, 1) &&
          !memcmp(&state, &saved, sizeof state));
    }
    struct fi_rotation s = {1, 0, 3, 7, 1, 0};
    C(fi_advance(&s) && fi_advance(&s) && fi_advance(&s));
    struct fi_rotation oldpolicy = s;
    C(!fi_advance(&s) && !memcmp(&s, &oldpolicy, sizeof s));
    for (unsigned cut = 0; cut < 6; cut++)
    {
        struct f4_storage store = {oldpolicy, cut, 0};
        C(!fi_rotate(&s, 2, 2, 8, 2, 1, persist, &store));
        C(!memcmp(&s, &oldpolicy, sizeof s) &&
          !memcmp(&store.durable, &oldpolicy, sizeof s) && store.calls == 1);
    }
    struct f4_storage store = {oldpolicy, 6, 0};
    C(!fi_rotate(&s, 2, 2, 8, 1, 1, persist, &store) && store.calls == 0);
    C(!fi_rotate(&s, 2, 2, 6, 2, 1, persist, &store) && store.calls == 0);
    C(fi_rotate(&s, 2, 2, 8, 2, 1, persist, &store));
    C(s.epoch == 2 && s.sequence == 0 && s.key == 2 && s.floor == 8 &&
      (s.revoked & 2) && !memcmp(&s, &store.durable, sizeof s));
    C(fi_advance(&s) && fi_advance(&s) && fi_advance(&s));
    oldpolicy = s;
    C(!fi_rotate(&s, 3, 1, 8, 1, 1, persist, &store) &&
      !memcmp(&s, &oldpolicy, sizeof s));
    s.epoch = UINT32_MAX;
    oldpolicy = s;
    C(!fi_rotate(&s, 0, 1, 8, 1, 1, persist, &store) &&
      !memcmp(&s, &oldpolicy, sizeof s));
    printf(
        "B06_IMAGE_PASS image_unit_cuts=%u recovery_commit_boundaries=7 revoked_key=denied epoch_wrap=denied\n",
        cuts);
    return 0;
}
