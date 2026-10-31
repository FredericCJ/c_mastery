#include "test_support.h"

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    const char *names[] = {"bank",     "order",   "lifecycle", "queue", "wake",
                           "deadline", "barrier", "fairness",  "permit"};
    void (*tests[])(void) = {test_bank,    test_order,    test_lifecycle,
                             test_queue,   test_wake,     test_deadline,
                             test_barrier, test_fairness, test_permit};
    for (size_t i = 0; i < sizeof names / sizeof names[0]; i++)
    {
        if (!strcmp(argv[1], names[i]))
        {
            tests[i]();
            return 0;
        }
    }
    return 2;
}
