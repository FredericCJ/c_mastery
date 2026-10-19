/* Compile only. No portable golden result and no execution requirement. */
#include <limits.h>
int fi_bad_add(int a,int b){return a+b;}
int fi_bad_divide(int a,int b){return a/b;}
int fi_bad_remainder(int a,int b){return a%b;}
int fi_bad_normalize(int a,int m){int r=a%m;return (r+m)%m;}
/* Caller contracts above are deliberately absent: signed overflow, divisor0,
 * unrepresentable quotient and normalization overflow are dossier cases. */
