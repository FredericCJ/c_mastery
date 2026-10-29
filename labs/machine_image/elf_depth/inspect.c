/* Hosted transport for a strict, allocation-free byte inspector. Each input is
 * uint32 little-endian length followed by that many bytes; stdout is JSONL.
 * Transport allocations/stdio are not target dependencies of ed_open. */
#include "elf_depth.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

static void hex(const char *p, size_t n)
{
    putchar('"');
    for (size_t i = 0; i < n; i++)
    {
        printf("%02x", (unsigned char)p[i]);
    }
    putchar('"');
}

int main(void)
{
    unsigned char *storage = malloc(16u * 1024u * 1024u + 1);
    if (!storage)
    {
        return 2;
    }
    unsigned char *bytes = storage + 1, header[4];
    for (;;)
    {
        size_t got = fread(header, 1, 4, stdin);
        if (!got && feof(stdin))
        {
            break;
        }
        if (got != 4)
        {
            free(storage);
            return 3;
        }
        uint32_t n = (uint32_t)header[0] | (uint32_t)header[1] << 8 |
                     (uint32_t)header[2] << 16 | (uint32_t)header[3] << 24;
        if (n > 16u * 1024u * 1024u || fread(bytes, 1, n, stdin) != n)
        {
            free(storage);
            return 3;
        }
        struct ed_view view;
        unsigned char before[sizeof view];
        memset(&view, 0xa5, sizeof view);
        memcpy(before, &view, sizeof view);
        enum ed_status status = ed_open(bytes, n, &view);
        printf("{\"status\":%u", (unsigned)status);
        if (status != ED_OK)
        {
            if (memcmp(before, &view, sizeof view))
            {
                free(storage);
                return 4;
            }
            puts("}");
            continue;
        }
        printf(",\"header\":[%u,%u,%u,%u,%" PRIu64 ",%u,%u,%u],\"sections\":[",
               view.bits, view.order, view.type, view.machine, view.entry,
               view.shnum, view.phnum, view.shstr);
        for (unsigned i = 0; i < view.shnum; i++)
        {
            struct ed_section s;
            if (ed_section(&view, i, &s) != ED_OK)
            {
                return 5;
            }
            printf("%s[%u,%u,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
                   ",%u,%u,%" PRIu64 ",%" PRIu64 "]",
                   i ? "," : "", s.name, s.type, s.flags, s.address, s.offset,
                   s.size, s.link, s.info, s.align, s.entsize);
        }
        printf("],\"programs\":[");
        for (unsigned i = 0; i < view.phnum; i++)
        {
            struct ed_program s;
            if (ed_program(&view, i, &s) != ED_OK)
            {
                return 5;
            }
            printf("%s[%u,%u,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64
                   ",%" PRIu64 ",%" PRIu64 "]",
                   i ? "," : "", s.type, s.flags, s.offset, s.address,
                   s.physical, s.filesz, s.memsz, s.align);
        }
        printf("],\"symbols\":[");
        unsigned total = 0;
        for (unsigned i = 0; i < view.shnum; i++)
        {
            struct ed_section section;
            ed_section(&view, i, &section);
            if (section.type != 2 && section.type != 11)
            {
                continue;
            }
            for (size_t j = 0; j < section.size / section.entsize; j++)
            {
                struct ed_symbol s;
                if (ed_symbol(&view, i, j, &s) != ED_OK)
                {
                    return 5;
                }
                const char *name = "";
                size_t length = 0;
                if (s.name && ed_name(&view, section.link, s.name, &name,
                                      &length) != ED_OK)
                {
                    return 5;
                }
                printf("%s[%u,%zu,%u,%u,%u,%u,%" PRIu64 ",%" PRIu64 ",",
                       total++ ? "," : "", i, j, s.name, s.info, s.other,
                       s.section, s.value, s.size);
                hex(name, length);
                putchar(']');
            }
        }
        puts("]}");
    }
    free(storage);
    return ferror(stdin) || ferror(stdout) ? 6 : 0;
}
