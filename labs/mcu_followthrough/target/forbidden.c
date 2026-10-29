extern int puts(const char *);

void forbidden_dependency(void)
{
    (void)puts("hosted I/O is not a target runtime provider");
}
