#include <limits.h>
#ifndef LC_CASE
#define LC_CASE 0
#endif
#if LC_CASE==1
constexpr volatile int value=3;
#elif LC_CASE==2
constexpr unsigned int value=-1;
#elif LC_CASE==3
constexpr int value;
#elif LC_CASE==4
constexpr int value=47;
enum { extent=value };
int main(void){return extent==47?0:1;}
#else
constexpr int value=42LL;
constexpr unsigned int maximum=-1U;
int main(void){return value==42&&maximum==UINT_MAX?0:1;}
#endif
