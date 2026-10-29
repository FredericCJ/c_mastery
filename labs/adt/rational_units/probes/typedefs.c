#include <stdint.h>
#include <stdio.h>
typedef int64_t adp_microseconds;
typedef int64_t adp_ticks;
static adp_microseconds consumes_us(adp_microseconds value){return value;}
int main(void){adp_ticks ticks=5;adp_microseconds result=consumes_us(ticks);if(result!=5)return 1;puts("ADQ_TYPEDEF substitution-accepted");return 0;}
