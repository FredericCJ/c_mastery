#include "adt.h"
int probe(void) { struct ad_millis value={7},out={0}; return ad_milliseconds(value,&out); }
