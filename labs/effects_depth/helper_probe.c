#include <stdint.h>
/* Pure-looking integer division can require a target compiler helper. */
uint64_t ef_divide(uint64_t value,uint64_t divisor){return divisor?value/divisor:0;}
