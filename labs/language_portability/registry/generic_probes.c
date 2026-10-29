#include LR_GENERIC_HEADER
#ifndef CASE
#error Define CASE
#endif
#if CASE == 1
int main(void){int a[3]={1,2,3};return LG_VALUE(a);}
#elif CASE == 2
#define BAD(x) _Generic((x),int:(x),default:(x).member)
int main(void){return BAD(3);}
#elif CASE == 3
typedef int lp_alias;
#define DUP(x) _Generic((x),int:lg_int,lp_alias:lg_int)((x))
int main(void){return DUP(3);}
#elif CASE == 4
int main(void){int a[4]={1,2,3,4};return LG_OBJECT(a);}
#elif CASE == 5
int generic_int(int x){return LG_VALUE(x);}
int explicit_int(int x){return lg_int(x);}
double generic_double(double x){return LG_VALUE(x);}
double explicit_double(double x){return lg_double(x);}
int main(void){return generic_int(3)==explicit_int(3) && generic_double(2.5)==explicit_double(2.5)?0:1;}
#elif CASE == 6
int main(void){return LG_EXTENDED(9L)==10L?0:1;}
#else
#error Unknown CASE
#endif
