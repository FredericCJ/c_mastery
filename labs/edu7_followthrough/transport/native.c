#include <stdlib.h>
int et_tests(unsigned);
int main(int n,char **a){return n==2?et_tests((unsigned)strtoul(a[1],0,10)):41;}
