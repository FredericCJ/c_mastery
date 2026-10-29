#include "once.h"
#include <stdio.h>
#include <limits.h>
#define CHECK(x)                                                   \
    do                                                             \
    {                                                              \
        if (!(x))                                                  \
        {                                                          \
            fprintf(stderr, "ONCE_X_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                              \
        }                                                          \
    } while (0)

int main(void)
{
    unsigned cases = 0;
    for (unsigned owner = 0; owner < 3; owner++)
        for (unsigned cut = 0; cut <= 9; cut++)
            for (int retry = 0; retry <= 1; retry++)
            {
                struct sx_journal journal = {0};
                struct sx_once o;
                unsigned table[8] = {99, 99, 99, 99, 99, 99, 99, 99},
                         checksum = 999;
                CHECK(sx_once_init(&o, &journal, 1, 77, retry) == MB_OK);
                CHECK(sx_once_enter(&o, owner, table, &checksum) == MB_INIT);
                CHECK(sx_once_enter(&o, owner, table, &checksum) ==
                      MB_RECURSIVE);
                CHECK(sx_once_enter(&o, (owner + 1) % 3, table, &checksum) ==
                      MB_BUSY);
                for (unsigned i = 0; i < cut; i++)
                    CHECK(sx_once_step(&o, owner) == MB_BUSY);
                CHECK(sx_once_cancel(&o, (owner + 1) % 3) == MB_STALE);
                CHECK(sx_once_cancel(&o, owner) == MB_OK &&
                      o.publications == 0 && checksum == 999 && table[0] == 99);
                CHECK(journal.effects == (cut != 0));
                if (retry)
                {
                    CHECK(sx_once_enter(&o, (owner + 1) % 3, table,
                                        &checksum) == MB_INIT);
                    for (unsigned i = 0; i < 9; i++)
                        CHECK(sx_once_step(&o, (owner + 1) % 3) == MB_BUSY);
                    CHECK(sx_once_step(&o, (owner + 1) % 3) == MB_OK &&
                          o.attempts == 2 && o.publications == 1 &&
                          journal.effects == 1);
                    CHECK(sx_once_enter(&o, owner, table, &checksum) == MB_OK &&
                          checksum == 484);
                    for (unsigned i = 0; i < 8; i++)
                        CHECK(table[i] == 17 * i + 1);
                    CHECK(sx_once_cancel(&o, (owner + 1) % 3) == MB_STALE);
                }
                else
                    CHECK(sx_once_enter(&o, (owner + 1) % 3, table,
                                        &checksum) == MB_FAILED &&
                          o.attempts == 1);
                cases++;
            }
    {
        struct sx_journal j = {0};
        struct sx_once a, b;
        unsigned t[8] = {0}, sum = 99;
        CHECK(sx_once_init(&a, &j, 1, 7, 1) == MB_OK &&
              sx_once_enter(&a, 0, t, &sum) == MB_INIT &&
              sx_once_step(&a, 0) == MB_BUSY && sx_once_cancel(&a, 0) == MB_OK);
        CHECK(sx_once_init(&b, &j, 1, 8, 0) == MB_OK &&
              sx_once_enter(&b, 1, t, &sum) == MB_INIT &&
              sx_once_step(&b, 1) == MB_FAILED && j.effects == 1);
    }
    {
        struct sx_journal j = {0};
        struct sx_once o;
        unsigned t[8] = {0}, sum = 99;
        CHECK(sx_once_init(&o, &j, 1, 7, 1) == MB_OK);
        o.attempts = UINT_MAX;
        CHECK(sx_once_enter(&o, 0, t, &sum) == MB_FULL && sum == 99);
    }
    printf("ONCE_X_PASS cuts=%u permanent_and_retry=1 dedup_effects=1\n",
           cases);
    return 0;
}
