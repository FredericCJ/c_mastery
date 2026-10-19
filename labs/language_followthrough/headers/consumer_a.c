#include <headers.h>
#include <limits.h>
HhFunction hh_a_address(void) { return hh_mix; }
unsigned *hh_a_state(void) { return hh_state(); }
unsigned hh_a_call(unsigned x) { return hh_mix(x); }
