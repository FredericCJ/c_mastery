static long wrong(const void *object, int *result)
{
    (void)object;
    *result = 1;
    return 0;
}

int (*required_callback)(const void *, int *) = wrong;
