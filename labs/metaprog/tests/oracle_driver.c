#include "metaprog.h"
#include <inttypes.h>
#include <stdio.h>

int main(void)
{
    char line[1024];
    while (fgets(line, sizeof line, stdin))
    {
        if (line[0] == 'H')
        {
            struct mp_key key;
            if (sscanf(line + 2, "%" SCNu32 " %" SCNu32 " %" SCNu32, &key.x,
                       &key.y, &key.cache) != 3)
            {
                return 2;
            }
            printf("%" PRIu64 "\n", mp_hash(key));
        }
        else if (line[0] == 'F')
        {
            uint32_t value;
            size_t cap, i;
            unsigned char buffer[32];
            struct mp_sink sink;
            int ok;
            if (sscanf(line + 2, "%" SCNu32 " %zu", &value, &cap) != 2 ||
                cap > 32)
            {
                return 2;
            }
            sink.bytes = buffer;
            sink.capacity = cap;
            sink.used = 0;
            ok = mp_format(&sink, mp_arg_number(value));
            printf("%d %zu", ok, sink.used);
            for (i = 0; i < sink.used; ++i)
            {
                printf(" %02x", buffer[i]);
            }
            puts("");
        }
        else if (line[0] == 'B')
        {
            uint64_t config;
            const unsigned char a[] = {1, 2}, b[] = {3};
            struct mp_build_node nodes[] = {{0, a, 2, 10, 20},
                                            {1, b, 1, 10, 20}};
            unsigned order[2];
            uint64_t keys[2];
            if (sscanf(line + 2, "%" SCNu64, &config) != 1)
            {
                return 2;
            }
            nodes[0].configuration = config;
            if (!mp_build(nodes, 2, order, keys))
            {
                return 3;
            }
            printf("%u %u %" PRIu64 " %" PRIu64 "\n", order[0], order[1],
                   keys[0], keys[1]);
        }
        else if (line[0] == 'E')
        {
            uint64_t keys[] = {UINT64_C(0), UINT64_C(0x123456789abcdef0),
                               UINT64_MAX};
            char output[256];
            size_t used;
            if (!mp_emit_table(keys, 3, output, sizeof output, &used))
            {
                return 3;
            }
            if (fwrite(output, 1, used, stdout) != used)
            {
                return 2;
            }
        }
        else
        {
            return 2;
        }
    }
    return ferror(stdin) ? 2 : 0;
}
