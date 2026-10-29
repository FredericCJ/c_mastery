#include "extension.h"
#include <stdio.h>
#include <string.h>
#include <limits.h>
#define CHECK(x)                                                         \
    do                                                                   \
    {                                                                    \
        if (!(x))                                                        \
        {                                                                \
            fprintf(stderr, "B06_EXT_CHECK line=%d %s\n", __LINE__, #x); \
            return 1;                                                    \
        }                                                                \
    } while (0)
static const char good[] =
    "fn twice(x){return x*2;} fn main(x){let i=0;let y=0;ptr p=&a[0];while(i<7){*p=twice(x+i);y=y+*p;p=p+1;i=i+1;}if(x<3){y=y-1;}else{y=y+2;}return y;}";

int main(int argc, char **argv)
{
    struct ce_program p, q;
    struct ce_diagnostic d;
    unsigned fuel, h[CE_CODE];
    uint32_t out = 99;
    char buf[131072];
    (void)argc;
    (void)argv;
    CHECK(ce_compile(good, strlen(good), &p, &d));
    CHECK(ce_verify(&p, h));
    fuel = 1000;
    CHECK(ce_run(&p, 2, &fuel, &out) && out == 69 && fuel < 1000);
    fuel = 1000;
    CHECK(ce_run(&p, UINT32_MAX, &fuel, &out) && out == 30);
    CHECK(ce_emit(&p, 0, buf, sizeof buf) && strstr(buf, "call ce_f0"));
    CHECK(ce_emit(&p, 1, buf, sizeof buf) && strstr(buf, "bl ce_f0"));
    buf[0] = 'Q';
    CHECK(!ce_emit(&p, 0, buf, 1) && buf[0] == 'Q');
    {
        unsigned char bytes[4252], again[4252];
        size_t n = 0, z = 0;
        CHECK(ce_pack(&p, bytes, sizeof bytes, &n));
        CHECK(ce_unpack(bytes, n, &q));
        CHECK(ce_pack(&q, again, sizeof again, &z) && n == z &&
              !memcmp(bytes, again, n));
        for (size_t cut = 0; cut < n; cut++)
        {
            q = p;
            CHECK(!ce_unpack(bytes, cut, &q) && !memcmp(&p, &q, sizeof p));
        }
        again[0] = 'Q';
        z = 99;
        CHECK(!ce_pack(&p, again, 1, &z) && z == 99 && again[0] == 'Q');
        memcpy(again, bytes, n);
        again[8 + 37 * p.functions + 1] = 1;
        CHECK(!ce_unpack(again, n, &q));
        memcpy(again, bytes, n);
        again[4] = 2;
        CHECK(!ce_unpack(again, n, &q));
    }
    q = p;
    fuel = 2;
    out = 99;
    CHECK(!ce_run(&p, 1, &fuel, &out) && fuel == 0 && out == 99);
    {
        const char *bad[] = {"fn main(x){return 0x;}",
                             "fn main(x){return 4294967296;}",
                             "fn main(x){return 12abc;}",
                             "fn main(x){return 0x12g;}",
                             "fn main(x){return x;} /*",
                             "fn main(x){ptr p=&a[0];return p;}",
                             "fn main(x){return main(x);}",
                             "fn main(x){return y;}",
                             "fn main(x){let y=1;let y=2;return y;}",
                             "fn main(x){if(x){let z=2;}return x;}",
                             "fn main(x){ptr p=&a[0];p=p*2;return x;}"};
        unsigned i;
        for (i = 0; i < sizeof bad / sizeof *bad; ++i)
        {
            p = q;
            CHECK(!ce_compile(bad[i], strlen(bad[i]), &p, &d));
            CHECK(!memcmp(&p, &q, sizeof p));
            CHECK(d.reason && d.begin <= d.end && d.end <= strlen(bad[i]));
        }
    }
    {
        const char *s = "fn main(x){ptr p=&a[8];return *p;}";
        CHECK(ce_compile(s, strlen(s), &p, &d));
        fuel = 20;
        out = 99;
        CHECK(!ce_run(&p, 0, &fuel, &out) && out == 99);
    }
    {
        const char *s =
            "/* h */fn main(x){// decimal and hex\nreturn 0xFFFFFFFF-x-1;}";
        CHECK(ce_compile(s, strlen(s), &p, &d));
        fuel = 20;
        CHECK(ce_run(&p, 1, &fuel, &out) && out == UINT32_MAX - 2);
    }
    {
        const char *s =
            "fn main(x){int y=i32(0)-i32(x);int z=y*i32(3);return u32(z)+1;}";
        CHECK(ce_compile(s, strlen(s), &p, &d));
        fuel = 100;
        CHECK(ce_run(&p, 7, &fuel, &out) && out == UINT32_MAX - 19);
        fuel = 100;
        out = 99;
        CHECK(!ce_run(&p, UINT32_MAX, &fuel, &out) && out == 99);
        fuel = 100;
        CHECK(!ce_run(&p, 1000000000, &fuel, &out));
    }
    {
        const char *s = "fn main(x){int n=i32(0)-i32(1);return n<1;}";
        CHECK(ce_compile(s, strlen(s), &p, &d));
        fuel = 50;
        CHECK(ce_run(&p, 0, &fuel, &out) && out == 0);
    }
    {
        const char *s = "fn main(x){int n=i32(0)-i32(1);return n<i32(1);}";
        CHECK(ce_compile(s, strlen(s), &p, &d));
        fuel = 50;
        CHECK(ce_run(&p, 0, &fuel, &out) && out == 1);
    }
    memset(&p, 0, sizeof p);
    p.functions = 1;
    p.count = 7;
    p.fn[0].end = 7;
    p.fn[0].locals = 1;
    p.fn[0].types[0] = CE_U32;
    p.code[0] = (struct ce_ins){CE_CONST, 0};
    p.code[1] = (struct ce_ins){CE_JZ, 5};
    p.code[2] = (struct ce_ins){CE_CONST, 0};
    p.code[3] = (struct ce_ins){CE_ADDR, 0};
    p.code[4] = (struct ce_ins){CE_JMP, 6};
    p.code[5] = (struct ce_ins){CE_CONST, 0};
    p.code[6] = (struct ce_ins){CE_RET, 0};
    CHECK(!ce_verify(&p, h));
    puts("B06_EXT_PASS parser typing control-flow calls pointers budget emit");
    return 0;
}
