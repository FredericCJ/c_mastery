#include <stdint.h>
#include <stdio.h>
typedef int64_t Microseconds;
typedef int64_t Ticks;
static Microseconds consumes_us(Microseconds value){return value;}
int main(void){Ticks ticks=5;Microseconds result=consumes_us(ticks);if(result!=5)return 1;puts("ADQ_TYPEDEF substitution-accepted");return 0;}
