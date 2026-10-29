#include "higher_order.h"

int main(void)
{
    struct he_ops ops = ho_eval_ops();
    struct he_fun function = {0};
    he_int out;
    return ops.add(function, 1, &out);
}
