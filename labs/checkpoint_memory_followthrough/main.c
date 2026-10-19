#include "checks.h"
#include <stdio.h>
int main(void) {
    unsigned error=p1_checks();
    if(error) {fprintf(stderr,"P1_CHECK line=%u\n",error);return 1;}
    puts("P1_CORE_PASS traces=6561 exact_fit one_short transactional_rejection retirement");return 0;
}
