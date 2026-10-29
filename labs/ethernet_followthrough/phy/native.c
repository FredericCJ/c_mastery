#include <stdio.h>

void ep_note(const char *s)
{
    puts(s);
}

int ep_access(void);
int ep_lifecycle(void);
int ep_bounded(void);
int ep_mapped(void);
int ep_leases(void);
int ep_faults(void);

int main(int argc, char **argv)
{
    if (argc != 2 || argv[1][1])
    {
        return 42;
    }
    switch (argv[1][0])
    {
        case '0':
            return ep_access();
        case '1':
            return ep_lifecycle();
        case '2':
            return ep_bounded();
        case '3':
            return ep_mapped();
        case '4':
            return ep_leases();
        case '5':
            return ep_faults();
        default:
            return 42;
    }
}
