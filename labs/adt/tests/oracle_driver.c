#include "adt.h"
#include <stdio.h>
#include <string.h>

static int nibble(char c)
{
    return c >= '0' && c <= '9'   ? c - '0'
           : c >= 'a' && c <= 'f' ? c - 'a' + 10
                                  : -1;
}

int main(void)
{
    char line[4096];
    while (fgets(line, sizeof line, stdin))
    {
        char kind = line[0];
        unsigned char data[1024];
        size_t n = 0, i;
        if (kind == 'T' || kind == 'P')
        {
            for (i = 2; line[i] && line[i] != '\n'; i += 2)
            {
                int a = nibble(line[i]), b = nibble(line[i + 1]);
                if (a < 0 || b < 0 || n >= sizeof data)
                {
                    return 2;
                }
                data[n++] = (unsigned char)(a * 16 + b);
            }
            if (kind == 'T')
            {
                struct ad_text out;
                int ok = ad_text(data, n, &out);
                if (ok)
                {
                    printf("1 %zu\n", out.scalars);
                }
                else
                {
                    puts("0");
                }
            }
            else
            {
                struct ad_packet out;
                enum ad_packet_error error = ad_packet(data, n, &out);
                printf("%u", (unsigned)error);
                if (error == AD_PACKET_OK)
                {
                    printf(" %u %zu %zu", out.kind, out.length,
                           (size_t)(out.payload - data));
                }
                puts("");
            }
        }
        else if (kind == 'V')
        {
            size_t r, c, rs, cs, cap;
            uint32_t backing[128];
            struct ad_view view;
            int ok;
            if (sscanf(line + 2, "%zu %zu %zu %zu %zu", &r, &c, &rs, &cs,
                       &cap) != 5 ||
                cap > 128)
            {
                return 2;
            }
            for (i = 0; i < 128; ++i)
            {
                backing[i] = (uint32_t)i;
            }
            ok = ad_view(backing, cap, r, c, rs, cs, &view);
            printf("%d", ok);
            if (ok)
            {
                size_t row, col;
                for (row = 0; row < r; ++row)
                {
                    for (col = 0; col < c; ++col)
                    {
                        uint32_t out;
                        if (!ad_view_get(view, row, col, &out))
                        {
                            return 3;
                        }
                        printf(" %u", out);
                    }
                }
            }
            puts("");
        }
        else
        {
            return 2;
        }
    }
    return ferror(stdin) ? 2 : 0;
}
