/* No prototype: int arguments/pointer retain their promoted types. */
int lp_old_sum();

int lp_old_consumer(int a, int b, int *out)
{
    return lp_old_sum(a, b, out);
}
