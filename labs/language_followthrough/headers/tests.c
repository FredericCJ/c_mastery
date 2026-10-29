#include <headers.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
hh_fn hh_a_address(void);
hh_fn hh_b_address(void);
unsigned *hh_a_state(void);
unsigned *hh_b_state(void);
unsigned hh_a_call(unsigned);
unsigned hh_b_call(unsigned);
#define REQUIRE(x) do { if (!(x)) { fprintf(stderr,"HH_ASSERT:%s\n",#x); return 41; } } while (0)
int main(int argc, char **argv)
{
    unsigned which = argc == 2 ? (unsigned)strtoul(argv[1], NULL, 10) : 99;
    if (which == 0) {
        int a[4] = {10,20,30,40}, out = 77;
        REQUIRE(hh_a_address() != hh_b_address());
        REQUIRE(hh_a_address()(7) == 22 && hh_b_call(7) == 22);
        REQUIRE(hh_a_call(UINT_MAX) == UINT_MAX - 1u);
        REQUIRE(hh_read(a, 4, 3, &out) == 0 && out == 40);
        REQUIRE(hh_read(a, 4, 4, &out) == -1 && out == 40);
        REQUIRE(hh_read(NULL, 0, 0, &out) == -1 && out == 40);
        REQUIRE(hh_read(a, 4, 0, NULL) == -1);
        puts("HH_DEFINITIONS_PASS"); return 0;
    }
    if (which == 1) {
        unsigned *a = hh_a_state(), *b = hh_b_state();
        REQUIRE(hh_a_call(7) == 22);
        REQUIRE(a != b && *a == 0 && *b == 0);
        *a = 7; REQUIRE(*b == 0); *b = 9;
        REQUIRE(hh_a_state() == a && *a == 7 && hh_b_state() == b && *b == 9);
        puts("HH_LOCAL_STATE_PASS"); return 0;
    }
    return 2;
}
