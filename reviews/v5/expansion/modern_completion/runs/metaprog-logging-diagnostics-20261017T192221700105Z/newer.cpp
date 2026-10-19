/* Comparison probe only: never part of the portable C17 implementation. */
#define FIRST(x,...) x __VA_OPT__(+ __VA_ARGS__)
static_assert(FIRST(2)==2,"empty tokens");
static_assert(FIRST(2,3)==5,"nonempty tokens");
int probe(void) { return FIRST(2,3); }

#include <stdio.h>
int main(void){if(probe()!=5)return 1;puts("MPL_VAOPT separate-mode empty-and-nonempty");return 0;}
