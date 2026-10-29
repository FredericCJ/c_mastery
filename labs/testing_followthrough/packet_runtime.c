#include <stddef.h>
#include <stdio.h>
int packet_public(const unsigned char *, size_t, unsigned *);

int main(void)
{
    unsigned char p[10] = {1, 4, 0, 1, 0, 2, 0, 3, 0, 4};
    const size_t lengths[] = {0, 1, 2, 9, 10, 11};
    for (unsigned i = 0; i < 6; i++)
    {
        unsigned out = 77;
        int ok = packet_public(p, lengths[i], &out);
        if (ok != (lengths[i] == 10) || out != (ok ? 10u : 77u))
            return 1;
    }
    unsigned out = 77;
    p[1] = 5;
    if (packet_public(p, 10, &out) || out != 77)
        return 1;
    p[1] = 4;
    p[0] = 2;
    if (packet_public(p, 10, &out) || out != 77)
        return 1;
    puts("PACKET_PASS:len0,1,2,9,10,11,count4,unchanged-failure,count5,opcode");
    return 0;
}
