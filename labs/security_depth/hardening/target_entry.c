#include "target.h"

_Noreturn void sh_entry(void)
{
    (void)sh_target_probe(7);
    __stack_chk_fail();
}
