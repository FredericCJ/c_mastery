#include "adt.h"
int probe(struct ad_seconds input) { struct ad_millis out={0}; return ad_milliseconds(input,&out); }
