#include "receivers.h"
int probe(volatile uint32_t *p,MpvRead *o){return mpv_read_m((uint32_t*)p,o);}
