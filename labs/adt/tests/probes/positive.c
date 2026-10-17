#include "adt.h"
int probe(ADSeconds input) { ADMillis out={0}; return ad_milliseconds(input,&out); }
