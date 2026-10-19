#ifndef CASE
#error Define CASE
#endif
#if CASE == 1
#include <stdio.h>
#define TRACE(...) printf(__VA_ARGS__)
int main(void){return TRACE();}
#elif CASE == 2
#include <stdio.h>
#define TRACE(format,...) printf(format,__VA_ARGS__)
int main(void){return TRACE("x");}
#elif CASE == 3
#include <stdio.h>
#define EMPTY
#define NESTED EMPTY
#define FORM(label,...) {label __VA_OPT__(,) __VA_ARGS__}
int main(void){int a[]=FORM(0),b[]=FORM(0,),c[]=FORM(0,EMPTY),d[]=FORM(0,NESTED),e[]=FORM(0,1,2);printf("%zu %zu %zu %zu %zu\n",sizeof a/sizeof *a,sizeof b/sizeof *b,sizeof c/sizeof *c,sizeof d/sizeof *d,sizeof e/sizeof *e);return 0;}
#elif CASE == 4
#define EMPTY
#define NESTED EMPTY
#define FORM(label,...) target(label,##__VA_ARGS__)
FORM(0)
FORM(0,)
FORM(0,EMPTY)
FORM(0,NESTED)
#elif CASE == 5
static int target(int x){return x;}
#define FORM(label,...) target(label,##__VA_ARGS__)
int main(void){return FORM(0);}
#elif CASE == 6
#define TWO() value=1;value=2
int main(void){int value=0;if(1)TWO();else value=3;return value;}
#elif CASE == 7
#define CAT_RAW(a,b) a##b
#define CAT(a,b) CAT_RAW(a,b)
#define DECL() int CAT(item_,__LINE__)=1
int main(void){DECL();DECL();return 0;}
#elif CASE == 8
#define CAT_RAW(a,b) a##b
#define CAT(a,b) CAT_RAW(a,b)
#define DECL() int CAT(item_,__COUNTER__)=1
int main(void){DECL();DECL();return item_0+item_1==2?0:1;}
#else
#error Unknown CASE
#endif
