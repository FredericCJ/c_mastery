#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
extern const uint64_t generated_keys[];

int main(void)
{
    printf("%" PRIu64 "\n", generated_keys[0]);
    return 0;
}
