/* Original negative audit specimen, never linked into the target core. */
#include <time.h>
time_t rpr_hidden_time(void){return time(NULL);}
