#include "adt.h"
int probe(void) { ADMillis value={7},out={0}; return ad_milliseconds(value,&out); }
