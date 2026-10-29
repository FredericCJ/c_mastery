#include "medium.h"
#include "services.h"
#include <stdio.h>
#include <string.h>
static struct vh_medium disk;
static store database;

int main(void)
{
    char kind;
    unsigned histories = 0, cases = 0;
    struct vg_vh h = {&database, 14, VH_VERSIONED5, 0, 0};
    while (scanf(" %c", &kind) == 1)
    {
        if (kind == 'B')
        {
            long long initial;
            int32_t out = 91;
            if (scanf("%lld", &initial) != 1)
            {
                return 2;
            }
            memset(&disk, 0, sizeof disk);
            if (store_format(vh_medium_io(&disk)) ||
                store_open(&database, vh_medium_io(&disk)) ||
                vh_host(&h, 1, (int32_t)initial, &out))
            {
                return 1;
            }
            histories++;
        }
        else if (kind == 'Q')
        {
            unsigned long long id, last;
            long long delta, expected, value;
            int ok;
            int32_t out = 91;
            if (scanf("%llu %lld %d %lld %lld %llu", &id, &delta, &ok,
                      &expected, &value, &last) != 6)
            {
                return 2;
            }
            unsigned before = disk.event;
            int rc = vh_increment(&h, id, (int32_t)delta, &out);
            if ((rc == STORE_OK) != ok || out != expected ||
                (!ok && disk.event != before))
            {
                return 1;
            }
            vh_medium_restart(&disk);
            if (store_open(&database, vh_medium_io(&disk)) ||
                store_check(&database) || vh_host(&h, 2, 0, &out) ||
                out != value)
            {
                return 1;
            }
            uint8_t bytes[64];
            size_t length = 0;
            rc = store_get(&database, "vm-operation", 12, bytes, sizeof bytes,
                           &length);
            if (!last)
            {
                if (rc != STORE_NOT_FOUND)
                {
                    return 1;
                }
            }
            else
            {
                if (rc || length != 17 || bytes[0] != 1)
                {
                    return 1;
                }
                uint64_t stored = 0;
                for (unsigned i = 0; i < 8; i++)
                {
                    stored |= (uint64_t)bytes[1 + i] << (8 * i);
                }
                if (stored != last)
                {
                    return 1;
                }
            }
            cases++;
        }
        else
        {
            return 2;
        }
    }
    if (histories != 648 || cases != 1944)
    {
        return 1;
    }
    puts("VH_ORACLE_PASS histories=648 cases=1944");
    return 0;
}
