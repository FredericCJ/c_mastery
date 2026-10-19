#include <limits.h>
/* ANALYSIS/COMPILATION ONLY. Volatile does not define signed overflow. */
int main(void) { volatile int x=INT_MAX; return x+1; }
