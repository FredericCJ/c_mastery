#include "depth.h"
#include <stdio.h>
#include <inttypes.h>

int main(void)
{
    char op;
    struct b2_map map;
    b2_map_init(&map);
    while (scanf(" %c", &op) == 1)
    {
        if (op == 'm')
        {
            char action;
            unsigned key;
            int value, out = 12345, r;
            if (scanf(" %c%u%d", &action, &key, &value) != 3 || key > 65535)
            {
                return 2;
            }
            if (action == 'p')
            {
                r = b2_map_put(&map, (uint16_t)key, value);
            }
            else if (action == 'r')
            {
                r = b2_map_remove(&map, (uint16_t)key);
            }
            else if (action == 'g')
            {
                r = b2_map_get(&map, (uint16_t)key, &out);
            }
            else
            {
                return 2;
            }
            printf("%d %zu %d\n", r, map.count, out);
        }
        else if (op == 'q')
        {
            int x, y;
            int8_t a, b, o = 99;
            if (scanf("%d%d", &x, &y) != 2 || x < -128 || x > 127 || y < -128 ||
                y > 127)
            {
                return 2;
            }
            a = (int8_t)x;
            b = (int8_t)y;
            int r = b2_q7(&a, &b, 1, &o);
            printf("%d %d\n", r, (int)o);
        }
        else if (op == 'p')
        {
            unsigned n, x, i;
            unsigned char bytes[32];
            int32_t out = 12345;
            if (scanf("%u", &n) != 1 || n > 32)
            {
                return 2;
            }
            for (i = 0; i < n; i++)
            {
                if (scanf("%u", &x) != 1 || x > 255)
                {
                    return 2;
                }
                bytes[i] = (unsigned char)x;
            }
            int r = b2_parse(bytes, n, &out);
            printf("%d %" PRId32 "\n", r, out);
        }
        else if (op == 's')
        {
            unsigned n, i;
            struct b2_item a[8], scratch[8];
            if (scanf("%u", &n) != 1 || n > 8)
            {
                return 2;
            }
            for (i = 0; i < n; i++)
            {
                if (scanf("%d", &a[i].key) != 1)
                {
                    return 2;
                }
                a[i].serial = i;
            }
            printf("%d", b2_sort(a, n, scratch, n));
            for (i = 0; i < n; i++)
            {
                printf(" %u", a[i].serial);
            }
            putchar('\n');
        }
        else
        {
            return 2;
        }
    }
    return 0;
}
