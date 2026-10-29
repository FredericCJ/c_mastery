#include "lc.h"
#include "test.h"

static void done(void *v, enum lc_status s)
{
    unsigned *p = v;
    *p = (unsigned)s + 1;
}

int main(void)
{
    /* Every permutation of request, driver step and acknowledgment; release is
       attempted after each event, but only terminal states authorize reclamation.
       No dangling pointer is invoked, even in the deliberately flawed variant. */
    const unsigned schedules[6][3] = {{0, 1, 2}, {0, 2, 1}, {1, 0, 2},
                                      {1, 2, 0}, {2, 0, 1}, {2, 1, 0}};
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned result = 0;
        struct lc_operation *p = lc_start(done, &result);
        CHECK(p);
        unsigned state = 0;
        for (unsigned j = 0; j < 3; j++)
        {
            unsigned event = schedules[i][j];
            enum lc_status expected;
            if (event == 0)
            {
                expected = state == 2 ? LC_OK : LC_PENDING;
                if (state != 2)
                    state = 1;
                CHECK(lc_cancel(p) == expected);
            }
            else if (event == 1)
            {
                expected = state == 1 ? LC_PENDING : LC_OK;
                if (!state)
                {
                    state = 2;
                    CHECK(!result);
                }
                CHECK(lc_poll(p) == expected);
            }
            else
            {
                expected = state == 0 ? LC_BUSY : LC_OK;
                if (state == 1)
                    state = 2;
                CHECK(lc_ack(p) == expected);
            }
            CHECK((result != 0) == (state == 2));
        }
        if (state != 2)
        {
            CHECK(lc_ack(p) == LC_OK);
            state = 2;
        }
        CHECK(lc_release_operation(p) == LC_OK);
    }
    puts(
        "LC_SCHEDULER six event orders; minimal premature-retirement witness is "
        "cancel before ack");
    return 0;
}
