#include "edition.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(e)                                                          \
    do                                                                    \
    {                                                                     \
        if (!(e))                                                         \
        {                                                                 \
            fprintf(stderr, "PE_ASSERT:%s:%u\n", #e, (unsigned)__LINE__); \
            exit(41);                                                     \
        }                                                                 \
    } while (0)

/* Independently written contract table. X means rejected, U means unknown
 * ownership, C retired, R only close may retry, B invalid owned handle. */
static void closes(void)
{
    struct pe_row
    {
        enum pe_version v;
        enum pe_call c;
        unsigned r;
        const char *expected;
    } rows[] = {{PE_2017, PE_CLOSE, 0, "CUXUBXX"},
                {PE_LINUX, PE_CLOSE, 0, "CCXCBXX"},
                {PE_2024, PE_CLOSE, 0, "CXCCBXX"},
                {PE_2024, PE_CLOSE, 1, "CRCCBXX"},
                {PE_2024, PE_PC_ZERO, 0, "CXCCBXX"},
                {PE_2024, PE_PC_ZERO, 1, "CXCCBXX"},
                {PE_2024, PE_PC_RESTART, 0, "CXCCBXX"},
                {PE_2024, PE_PC_RESTART, 1, "CRCCBXX"},
                {PE_2024, PE_PC_INVALID, 0, "CXCCBCX"},
                {PE_2024, PE_PC_INVALID, 1, "CXCCBCX"}};

    for (unsigned i = 0; i < sizeof rows / sizeof *rows; ++i)
    {
        for (unsigned j = 0; j < 7; ++j)
        {
            struct pe_decision d = {PE_UNRESOLVED, 37};
            char e = rows[i].expected[j];
            int result = pe_close(rows[i].v, rows[i].c, rows[i].r,
                                  (enum pe_outcome)j, &d);
            if (e == 'X')
            {
                CHECK(result == 1 && d.action == PE_UNRESOLVED &&
                      d.diagnostic == 37);
            }
            else
            {
                enum pe_action a = e == 'C'   ? PE_RETIRED
                                   : e == 'R' ? PE_RETRY_CLOSE_ONLY
                                   : e == 'B' ? PE_INVALID_HANDLE
                                              : PE_UNRESOLVED;
                CHECK(result == 0 && d.action == a && d.diagnostic == (j != 0));
            }
        }
    }
    struct pe_decision d = {PE_UNRESOLVED, 37};
    CHECK(pe_close(PE_2017, PE_PC_ZERO, 0, PE_OK, &d) == 1 &&
          d.diagnostic == 37);
    CHECK(pe_close(PE_LINUX, PE_PC_RESTART, 0, PE_OK, &d) == 1 &&
          d.diagnostic == 37);
    CHECK(pe_close(PE_2024, PE_CLOSE, 2, PE_OK, &d) == 1 && d.diagnostic == 37);
    CHECK(pe_close(PE_2024, PE_CLOSE, 0, PE_OK, 0) == 1);
    CHECK(pe_close((enum pe_version)99, PE_CLOSE, 0, PE_OK, &d) == 1 &&
          d.diagnostic == 37);
    puts("PE_CLOSE 70_table_cells 5_boundary_rejections");
}

static void reads(void)
{
    struct pe_row
    {
        unsigned a, b, c, d, e, f, g;
        enum pe_read result;
    } rows[] = {{0, 0, 0, 0, 0, 0, 0, PE_UNDEFINED},
                {1, 1, 0, 0, 0, 0, 0, PE_MAY_DEADLOCK},
                {1, 0, 1, 0, 0, 0, 0, PE_BLOCK},
                {1, 0, 0, 0, 0, 0, 0, PE_ACQUIRE},
                {1, 0, 0, 0, 1, 0, 0, PE_IMPL_CHOICE},
                {1, 0, 0, 1, 1, 0, 1, PE_IMPL_CHOICE},
                {1, 0, 0, 0, 1, 1, 1, PE_BLOCK},
                {1, 0, 0, 1, 1, 1, 1, PE_ACQUIRE},
                {1, 0, 0, 0, 1, 1, 0, PE_ACQUIRE},
                {1, 0, 0, 1, 1, 1, 0, PE_ACQUIRE}};

    for (unsigned i = 0; i < sizeof rows / sizeof *rows; ++i)
    {
        struct pe_row *r = &rows[i];
        enum pe_read out = PE_UNDEFINED;
        CHECK(pe_read(r->a, r->b, r->c, r->d, r->e, r->f, r->g, &out) == 0 &&
              out == r->result);
    }
    enum pe_read out = PE_UNDEFINED;
    CHECK(pe_read(1, 0, 1, 1, 1, 1, 1, &out) == 1 && out == PE_UNDEFINED);
    CHECK(pe_read(1, 0, 0, 0, 0, 1, 1, &out) == 1 && out == PE_UNDEFINED);
    CHECK(pe_read(2, 0, 0, 0, 0, 0, 0, &out) == 1 && out == PE_UNDEFINED);
    CHECK(pe_read(1, 0, 0, 0, 0, 0, 0, 0) == 1);
    puts("PE_READ 10_contract_rows 4_boundary_rejections");
}

int main(int argc, char **argv)
{
    CHECK(argc == 2);
    if (!strcmp(argv[1], "close"))
    {
        closes();
    }
    else if (!strcmp(argv[1], "read"))
    {
        reads();
    }
    else
    {
        return 2;
    }
    return 0;
}
