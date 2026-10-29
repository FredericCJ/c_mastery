#include "__fc_builtin.h"
#ifndef EVA_MIN
#define EVA_MIN -1
#endif
int main(void)
{
    int x = Frama_C_interval(EVA_MIN, 2);
    return 12 / (x + 1);
}
