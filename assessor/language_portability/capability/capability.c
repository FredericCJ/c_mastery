#include "capability.h"
#include <limits.h>
#include <inttypes.h>
#include <stdio.h>
int lp_record(unsigned present, intmax_t value, struct lp_value *out) {
    if (!out || present > 1) return 0;
    *out = (struct lp_value){.present=present, .value=(present || LP_MUTANT==4) ? value : 0};
    return 1;
}
int lp_format(struct lp_value value, char *out, size_t capacity) {
    char staged[128];
    int n;
    if (!out || value.present > 1) return 0;
    if (value.present) n=snprintf(staged,sizeof staged,"%" PRIdMAX,value.value);
    else n=snprintf(staged,sizeof staged,"empty");
    if (n<0 || (size_t)n>=sizeof staged || (size_t)n>=capacity) return 0;
    for (size_t i=0;i<=(size_t)n;i++) out[i]=staged[i];
    return 1;
}
int lp_transform(const int *restrict source, int *restrict destination, size_t n) {
    if (n>16 || (n && (!source || !destination))) return 0;
    for (size_t i=0;i<n;i++) {
        if (source[i]>INT_MAX/2 || source[i]<INT_MIN/2) return 0;
#if LP_MUTANT == 1
        destination[i]=source[i]*2;
#endif
    }
    for (size_t i=0;i<n;i++) destination[i]=source[i]*2;
    return 1;
}
int lp_capability(unsigned required, unsigned observed, unsigned *usable) {
    if (!usable || required>15 || observed>15) return 0;
#if LP_MUTANT == 2
    *usable=(observed & 1u)!=0;
#else
    *usable=(required & observed)==required;
#endif
    return 1;
}
