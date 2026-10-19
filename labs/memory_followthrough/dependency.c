#include <stddef.h>
/* Original local capability control. Compile separately with/without ASan. */
void mw_dependency(unsigned char *p,size_t i){p[i]=7;}
void mw_missing_field(unsigned *p,unsigned fixed){if(fixed)*p=1;}
