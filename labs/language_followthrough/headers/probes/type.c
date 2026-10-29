int hh_pointer(const int *);

int hh_bad_type(void)
{
    return hh_pointer(7);
}
