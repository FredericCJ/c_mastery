#include "higher_order.h"

int main(void)
{
    struct he_ops ops = ho_eval_ops();
    struct hp_int wrong = {{0}, 0};
    he_int out;
    return ops.add(wrong, 1, &out);
}
