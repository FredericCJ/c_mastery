#include "systems_certificates.h"
#include <stdio.h>
#include <string.h>
#define CHECK(c)                                               \
    do                                                         \
    {                                                          \
        if (!(c))                                              \
        {                                                      \
            fprintf(stderr, "SC_CHECK:%d:%s\n", __LINE__, #c); \
            return 1;                                          \
        }                                                      \
    } while (0)

struct sc_port
{
    int present, calls;
};

static int contains(void *p, int32_t key)
{
    (void)key;
    return ((struct sc_port *)p)->present;
}

static int insert(void *p, int32_t key)
{
    (void)key;
    ((struct sc_port *)p)->calls++;
    ((struct sc_port *)p)->present = 1;
    return 0;
}

static int tree(void)
{
    struct sc_tree t = {0};
    unsigned h = 91;
    struct sc_port port = {1, 0};
    t.root = 0;
    t.count = 3;
    t.balanced = 1;
    t.nodes[0] = (struct sc_node){20, 1, 2, -1, 2, 1};
    t.nodes[1] = (struct sc_node){10, -1, -1, 0, 1, 1};
    t.nodes[2] = (struct sc_node){30, -1, -1, 0, 1, 1};
    CHECK(sc_tree_check(&t, &h) == SC_OK && h == 2);
    t.nodes[0].height = 3;
    h = 91;
    CHECK(sc_tree_check(&t, &h) == SC_INVALID && h == 91);
    t.nodes[0].height = 2;
    t.nodes[1].left = 0;
    CHECK(sc_tree_check(&t, &h) == SC_INVALID);
    t.nodes[1].left = -1;
    t.nodes[2].parent = 1;
    CHECK(sc_tree_check(&t, &h) == SC_INVALID);
    t.nodes[2].parent = 0;
    t.nodes[2].key = 20;
    CHECK(sc_tree_check(&t, &h) == SC_INVALID);
    CHECK(sc_unique_insert(&port, contains, insert, 1) == SC_DUPLICATE &&
          port.calls == 0);
    port.present = 0;
    CHECK(sc_unique_insert(&port, contains, insert, 1) == SC_OK &&
          port.calls == 1);
    return 0;
}

static int matches(void)
{
    uint8_t text[] = {0, 1, 0, 1, 0}, pattern[] = {1, 0}, out[65];
    size_t n = 91;
    memset(out, 99, sizeof out);
    CHECK(sc_matches(text, 5, pattern, 2, out, 65, &n) == SC_OK && n == 2 &&
          out[0] == 1 && out[1] == 3);
    CHECK(sc_matches((const uint8_t *)"aaaa", 4, (const uint8_t *)"aa", 2, out,
                     65, &n) == SC_OK &&
          n == 3 && out[0] == 0 && out[1] == 1 && out[2] == 2);
    CHECK(sc_matches(text, 5, NULL, 0, out, 65, &n) == SC_OK && n == 6 &&
          out[5] == 5);
    out[0] = 99;
    n = 91;
    CHECK(sc_matches(text, 5, NULL, 0, out, 5, &n) == SC_FULL && out[0] == 99 &&
          n == 91);
    CHECK(sc_matches(NULL, 0, NULL, 0, out, 1, &n) == SC_OK && n == 1 &&
          out[0] == 0);
    return 0;
}

static int clock_test(void)
{
    struct sc_machine m = {0}, before;
    struct sc_control c = {0}, old;
    unsigned n = 91;
    struct sc_instruction code[] = {{SC_LI, 1, 3, 0},  {SC_LI, 2, 1, 0},
                                    {SC_SUB, 1, 1, 2}, {SC_BNZ, 1, 2, 0},
                                    {SC_ST, 1, 5, 0},  {SC_LD, 3, 5, 0},
                                    {SC_HALT, 0, 0, 0}};
    CHECK(sc_run(&m, code, 7, 4, &n) == SC_BUDGET && n == 4 && m.reg[1] == 2 &&
          m.pc == 2);
    CHECK(sc_run(&m, code, 7, 7, &n) == SC_DONE && n == 7 && m.commits == 11 &&
          m.reg[1] == 0 && m.reg[2] == 1 && m.halted && m.pc == 6);
    before = m;
    old = c;
    CHECK(sc_clock(&m, code, 7, &c) == SC_DONE &&
          !memcmp(&m, &before, sizeof m) && !memcmp(&c, &old, sizeof c));
    m = (struct sc_machine){0};
    m.pc = 7;
    n = 91;
    before = m;
    CHECK(sc_run(&m, code, 7, 0, &n) == SC_INVALID && n == 91 &&
          !memcmp(&m, &before, sizeof m));
    m = (struct sc_machine){0};
    n = 91;
    CHECK(sc_run(&m, code, 7, 0, &n) == SC_BUDGET && n == 0 && !m.commits &&
          !m.halted);
    m = (struct sc_machine){0};
    code[0] = (struct sc_instruction){SC_SUB, 1, 1, 2};
    m.reg[1] = 9;
    m.reg[2] = 3;
    CHECK(sc_clock(&m, code, 7, &c) == SC_OK && m.reg[1] == 6 && c.alu == 6 &&
          c.reg_write == 2);
    code[1] = (struct sc_instruction){SC_LD, 3, 16, 0};
    before = m;
    old = c;
    CHECK(sc_clock(&m, code, 7, &c) == SC_INVALID &&
          !memcmp(&m, &before, sizeof m) && !memcmp(&c, &old, sizeof c));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
        return 2;
    if (!strcmp(argv[1], "tree"))
        return tree();
    if (!strcmp(argv[1], "matches"))
        return matches();
    if (!strcmp(argv[1], "clock"))
        return clock_test();
    return 2;
}
