#include "owning_option.h"
#if EOP_DIAGNOSTIC == 1
struct invalid_member {void payload;};
#elif EOP_DIAGNOSTIC == 2
static void no_value(void){}
int main(void){EopUnit u=no_value();return eop_unit(u);}
#elif EOP_DIAGNOSTIC == 3
_Noreturn void bad_return(void){return;}
#else
/* This body is never invoked. A function specifier is not a stored bottom type. */
_Noreturn void permanent_loop(void){for(;;){}}
int main(void){EopUnit u=EOP_UNIT;return eop_unit(u);}
#endif
