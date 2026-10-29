/* Source-comparison regression for the explicitly narrower teaching profile. */
#include "root_checks.h"
#include <stdio.h>

int main(void)
{
    unsigned char madt[52], root[44], rsdp[40] = {0};
    struct sa_topology out = {0}, before;
    sa_base(madt, 52);
    madt[44] = 0;
    madt[45] = 8;
    madt[46] = 17;
    madt[47] = 6;
    sa_w32(madt + 48, 1);
    sa_sum(madt, 52);
    sa_base(root, 44);
    root[0] = 'X';
    root[1] = 'S';
    root[2] = 'D';
    root[3] = 'T';
    sa_w32(root + 36, 0x4000);
    sa_sum(root, 44);
    for (unsigned i = 0; i < 8; ++i)
    {
        rsdp[i] = (unsigned char)"RSD PTR "[i];
    }
    rsdp[15] = 2;
    sa_w32(rsdp + 20, 36);
    sa_w32(rsdp + 24, 0x2000);
    sa_checksum_at(rsdp, 20, 8);
    sa_checksum_at(rsdp, 36, 32);
    struct sa_region regions[] = {{0x2000, root, 44}, {0x4000, madt, 52}};
    if (sa_checks() || sa_root_checks())
    {
        return 1;
    }
    if (sa_discover(rsdp, 36, regions, 2, 16, 0, &out) != SA_OK)
    {
        return 2;
    }
    before = out;
    rsdp[15] = 3;
    sa_checksum_at(rsdp, 20, 8);
    sa_checksum_at(rsdp, 36, 32);
    if (sa_discover(rsdp, 36, regions, 2, 16, 0, &out) != SA_UNSUPPORTED ||
        !sa_same(&before, &out, sizeof out))
    {
        return 3;
    }
    rsdp[15] = 2;
    sa_w32(rsdp + 20, 40);
    sa_checksum_at(rsdp, 20, 8);
    sa_checksum_at(rsdp, 40, 32);
    if (sa_discover(rsdp, 40, regions, 2, 16, 0, &out) != SA_INVALID ||
        !sa_same(&before, &out, sizeof out))
    {
        return 4;
    }
    sa_w32(rsdp + 20, 36);
    sa_checksum_at(rsdp, 20, 8);
    sa_checksum_at(rsdp, 36, 32);
    rsdp[7] = '!';
    sa_checksum_at(rsdp, 20, 8);
    sa_checksum_at(rsdp, 36, 32);
    if (sa_discover(rsdp, 36, regions, 2, 16, 0, &out) != SA_INVALID)
    {
        return 5;
    }
    rsdp[7] = ' ';
    sa_checksum_at(rsdp, 20, 8);
    sa_checksum_at(rsdp, 36, 32);
    rsdp[24] ^= 1;
    if (sa_discover(rsdp, 36, regions, 2, 16, 0, &out) != SA_INVALID ||
        !sa_same(&before, &out, sizeof out))
    {
        return 6;
    }
    puts("SA_HEADER_SUCCESSOR:strict-profile-and-two-checksums passed");
    return 0;
}
