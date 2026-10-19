typedef unsigned long long U;
typedef struct {U sum,first_xor_last,last;} Result;
__attribute__((noinline)) Result po_aggregate(U a,U b,U c,U d,U e,U f,U g,U h){Result r={a+b+c+d+e+f+g+h,a^h,h};return r;}
