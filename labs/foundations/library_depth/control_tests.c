#include "control.h"
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x)                                                        \
    do                                                                  \
    {                                                                   \
        if (!(x))                                                       \
        {                                                               \
            fprintf(stderr, "CONTROL-CHECK:%s line%d\n", #x, __LINE__); \
            return 1;                                                   \
        }                                                               \
    } while (0)

int main(void)
{
    CHECK(lg_raise() == LG_OK);
    for (unsigned mode = 0; mode < 4; ++mode)
    {
        struct lg_owner owner = {0};
        int changed = 99;
        enum lg_status r = lg_jump(&owner, mode, &changed);
        if (owner.allocation)
        {
            free(owner.allocation);
            owner.allocation = NULL;
            CHECK(owner.acquired == owner.released);
        }
        CHECK(r == (mode == 2 ? LG_FAILURE : LG_OK) && changed == 7 &&
              owner.acquired == (mode == 2 ? 0 : 1) &&
              owner.released == owner.acquired);
    }
    struct lg_publication p;
    lg_init(&p);
    int value = 99;
    CHECK(lg_receive(&p, &value) == LG_PENDING && value == 99);
    lg_publish(&p, 37);
    CHECK(lg_receive(&p, &value) == LG_OK && value == 37);
    CHECK(lg_receive(&p, &value) == LG_OK && value == 37);
    printf(
        "CONTROL-PASS:single-thread raise,4 ownership/nonlocal modes,one-shot "
        "publication;atomic_int_lock_free=%d\n",
        atomic_is_lock_free(&p.ready));
    return 0;
}
