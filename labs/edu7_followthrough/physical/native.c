#include <stdlib.h>
int ep_tests(unsigned);
int main(int n,char **a){return n==2?ep_tests((unsigned)strtoul(a[1],0,10)):41;}
