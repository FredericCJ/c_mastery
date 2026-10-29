#include "runtime.h"
unsigned char old_space[16], new_space[16];
unsigned polls, slots, mutant;

int main(int argc, char **argv)
{
    unsigned offset, i;
    mutant = argc > 1 ? (unsigned)atoi(argv[1]) : 0;
    for (offset = 0; offset < 8; offset++)
    {
        uint64_t scalar = (uint64_t)(uintptr_t)old_space, result;
        for (i = 0; i < 8; i++)
        {
            old_space[i] = (unsigned char)(10 + i);
        }
        result = sp_entry(old_space, offset, scalar);
        CHECK(result == scalar + 27 + offset);
        CHECK(slots >= 1 && polls == offset + 1);
    }
    printf("STATEPOINT_OK polls=%u slots=%u scalar_preserved=1\n", polls,
           slots);
    return 0;
}
