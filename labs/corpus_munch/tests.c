#include "dfa_cases.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "MM_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

static int semantics(void)
{
    struct mm_dfa d = mm_example();
    struct mm_workspace w;
    struct mm_tokens out;
    struct mm_cost c;
    unsigned err = 999;
    for (unsigned mode = 0; mode < 2; ++mode)
    {
        CHECK(mm_scan(&d, (const unsigned char *)"abcabcd", 7, mode, 256, &w,
                      &out, &c, &err) == PD_OK);
        CHECK(out.count == 1 && out.token[0].begin == 0 &&
              out.token[0].end == 7 && out.token[0].rule == 1);
        CHECK(mm_scan(&d, (const unsigned char *)"abc", 3, mode, 256, &w, &out,
                      &c, &err) == PD_OK);
        CHECK(out.count == 1 && out.token[0].rule == 0);
        CHECK(mm_scan(&d, (const unsigned char *)"dabc", 4, mode, 256, &w, &out,
                      &c, &err) == PD_OK);
        CHECK(out.count == 2 && out.token[0].rule == 1 &&
              out.token[1].rule == 0);
        CHECK(mm_scan(&d, NULL, 0, mode, 0, &w, &out, &c, &err) == PD_OK &&
              out.count == 0);
    }
    {
        unsigned char byte = 255;
        for (unsigned bit = 0; bit < 8; ++bit)
        {
            d.accept[7] = (unsigned char)(1u << bit);
            CHECK(mm_scan(&d, (const unsigned char *)"d", 1, 1, 1, &w, &out, &c,
                          &err) == PD_OK &&
                  out.token[0].rule == bit);
        }
        d.map[255] = 3;
        CHECK(mm_scan(&d, &byte, 1, 1, 1, &w, &out, &c, &err) == PD_OK &&
              out.token[0].rule == 7);
    }
    return 0;
}

static int bounds(void)
{
    struct mm_dfa d = mm_example(), bad;
    struct mm_workspace w;
    struct mm_tokens out, saved;
    struct mm_cost c, old;
    unsigned err = 999;
    CHECK(mm_scan(&d, (const unsigned char *)"abcabc", 6, 0, 2, &w, &out, &c,
                  &err) == PD_OK);
    CHECK(out.count == 2 && out.token[0].end == 3 && out.token[1].begin == 3 &&
          out.token[1].end == 6 && c.rollback_bytes == 3);
    memset(&out, 0x5a, sizeof out);
    saved = out;
    memset(&c, 0xa5, sizeof c);
    old = c;
    for (unsigned mode = 0; mode < 2; ++mode)
    {
        CHECK(mm_scan(&d, (const unsigned char *)"abcabc", 6, mode, 1, &w, &out,
                      &c, &err) == PD_RANGE &&
              err == 3);
        CHECK(!memcmp(&out, &saved, sizeof out) && !memcmp(&c, &old, sizeof c));
        CHECK(mm_scan(&d, (const unsigned char *)"abcx", 4, mode, 256, &w, &out,
                      &c, &err) == PD_NO_MATCH &&
              err == 3);
        CHECK(!memcmp(&out, &saved, sizeof out) && !memcmp(&c, &old, sizeof c));
        CHECK(mm_scan(&d, (const unsigned char *)"ab", 2, mode, 256, &w, &out,
                      &c, &err) == PD_NO_MATCH &&
              err == 0);
    }
    bad = d;
    bad.accept[0] = 1;
    CHECK(mm_scan(&bad, NULL, 0, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    bad = d;
    bad.next[1][1] = 16;
    CHECK(mm_scan(&bad, NULL, 0, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    bad = d;
    bad.map[255] = 5;
    CHECK(mm_scan(&bad, NULL, 0, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    bad = d;
    bad.start = 16;
    CHECK(mm_scan(&bad, NULL, 0, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    bad = d;
    bad.count = 0;
    CHECK(mm_scan(&bad, NULL, 0, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    bad = d;
    bad.count = 17;
    CHECK(mm_scan(&bad, NULL, 0, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    CHECK(mm_scan(&d, NULL, 1, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    CHECK(mm_scan(NULL, NULL, 0, 0, 1, &w, &out, &c, &err) == PD_INVALID);
    CHECK(mm_scan(&d, NULL, 0, 0, 1, &w, NULL, &c, &err) == PD_INVALID);
    CHECK(mm_scan(&d, NULL, 0, 0, 1, &w, &out, NULL, &err) == PD_INVALID);
    CHECK(mm_scan(&d, NULL, 0, 0, 257, &w, &out, &c, &err) == PD_INVALID);
    CHECK(mm_scan(&d, NULL, 0, 1, 1, NULL, &out, &c, &err) == PD_INVALID);
    CHECK(mm_scan(&d, (const unsigned char *)"d", 257, 0, 256, &w, &out, &c,
                  &err) == PD_INVALID);
    CHECK(mm_scan(&d, NULL, 0, 2, 256, &w, &out, &c, &err) == PD_INVALID);
    return 0;
}

static int memo(void)
{
    struct mm_dfa d = mm_example();
    struct mm_workspace w;
    struct mm_tokens a, b;
    struct mm_cost ca, cb;
    unsigned char text[256];
    unsigned err = 999;
    for (unsigned i = 0; i < 255; ++i)
    {
        text[i] = (unsigned char)"abc"[i % 3];
    }
    memset(&w, 0xff, sizeof w);
    CHECK(mm_scan(&d, text, 255, 0, 256, &w, &a, &ca, &err) == PD_OK);
    CHECK(mm_scan(&d, text, 255, 1, 256, &w, &b, &cb, &err) == PD_OK);
    CHECK(a.count == 85 && b.count == a.count &&
          ca.transitions == 3 * 85 * 86 / 2);
    CHECK(cb.transitions < 3 * 255 && cb.memo_stops > 0 &&
          cb.marked_pairs > 0 && cb.transitions < ca.transitions);
    for (unsigned i = 0; i < a.count; ++i)
    {
        CHECK(a.token[i].begin == b.token[i].begin &&
              a.token[i].end == b.token[i].end &&
              a.token[i].rule == b.token[i].rule);
    }
    /* Same scratch, new input and accepting-state labels must not reuse failures. */
    text[255] = 'd';
    CHECK(mm_scan(&d, text, 256, 1, 1, &w, &b, &cb, NULL) == PD_OK &&
          b.count == 1 && b.token[0].end == 256);
    d.accept[6] = 8;
    CHECK(mm_scan(&d, text, 255, 1, 256, &w, &b, &cb, NULL) == PD_OK &&
          b.count == 1 && b.token[0].rule == 3);
    {
        struct rx_nfa nfa;
        struct rx_dfa prior;
        struct pd_diagnostic diag;
        struct mm_dfa imported;
        CHECK(rx_compile((const unsigned char *)"a|ab", 4, &nfa, &diag) ==
              PD_OK);
        CHECK(rx_determinize(&nfa, 16, &prior) == PD_OK &&
              mm_from_rx(&prior, &imported) == PD_OK);
        CHECK(mm_scan(&imported, (const unsigned char *)"aba", 3, 1, 256, &w,
                      &b, &cb, NULL) == PD_OK &&
              b.count == 2 && b.token[0].end == 2 && b.token[1].end == 3);
        {
            struct mm_dfa saved = imported;
            prior.accept[0] = 1;
            CHECK(mm_from_rx(&prior, &imported) == PD_INVALID &&
                  !memcmp(&saved, &imported, sizeof saved));
            prior.accept[0] = 0;
            prior.next[0][0] = prior.count;
            CHECK(mm_from_rx(&prior, &imported) == PD_INVALID &&
                  !memcmp(&saved, &imported, sizeof saved));
            CHECK(mm_from_rx(NULL, &imported) == PD_INVALID &&
                  !memcmp(&saved, &imported, sizeof saved));
        }
    }
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    if (!strcmp(argv[1], "semantics"))
    {
        return semantics();
    }
    if (!strcmp(argv[1], "bounds"))
    {
        return bounds();
    }
    if (!strcmp(argv[1], "memo"))
    {
        return memo();
    }
    return 2;
}
