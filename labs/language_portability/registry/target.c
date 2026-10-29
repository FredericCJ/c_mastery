#include "registry.h"
static volatile uint32_t ids[] = {1, 4, 7, 3};
static volatile unsigned scenarios[] = {0, 1, 2, 3, 4, 5, 6, 7, 8};

int lr_target_main(void)
{
    for (unsigned round = 0; round < 32; ++round)
    {
        for (unsigned i = 0; i < 4; ++i)
        {
            int value = 77;
            uint32_t id = ids[i];
            int status = lr_dispatch(id, 10, &value);
            if (i < 3)
            {
                if (status || value != 10 + (int)id)
                {
                    return 1;
                }
            }
            else if (status != -1 || value != 77)
            {
                return 2;
            }
        }
        for (unsigned i = 0; i < 9; ++i)
        {
            struct lr_generic r;
            if (lr_generic(scenarios[i], &r))
            {
                return 3;
            }
            if (i == 1 && r.real != 3.5)
            {
                return 4;
            }
            if (i == 6 && (r.value != 4 || r.evaluated != 1))
            {
                return 5;
            }
        }
    }
    return 0;
}
