#include <stdio.h>
#ifdef LC_HEADER
#include <stdbool.h>
#endif
int main(void){bool value=true;
#ifdef bool
 puts("BOOL_MACRO");
#else
 puts("BOOL_NONMACRO");
#endif
 return value?0:1;}
