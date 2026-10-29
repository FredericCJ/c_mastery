typedef unsigned (*hh_experiment_fn)(unsigned);
unsigned hh_experiment_a(unsigned);
unsigned hh_experiment_b(unsigned);
#if !HH_CALL_ONLY
hh_experiment_fn hh_experiment_address_a(void);
hh_experiment_fn hh_experiment_address_b(void);
#endif
int main(void)
{
    if (hh_experiment_a(7) != 22 || hh_experiment_b(9) != 28)
    {
        return 41;
    }
#if !HH_CALL_ONLY
    if (hh_experiment_address_a() != hh_experiment_address_b())
    {
        return 42;
    }
    if (hh_experiment_address_a()(3) != 10)
    {
        return 43;
    }
#endif
    return 0;
}
