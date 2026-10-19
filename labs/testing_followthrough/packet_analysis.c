/* Eva entry models are deliberately separate from public runtime admission. */
#include <stddef.h>
#ifdef __FRAMAC__
#include "__fc_builtin.h"
#endif
#ifndef ANALYSIS_CASE
#define ANALYSIS_CASE 0
#endif
volatile unsigned analysis_sink;
static unsigned unchecked(const unsigned char *p,unsigned count){unsigned sum=0;for(unsigned i=0;i<count;i++)sum+=((unsigned)p[2+2*i]<<8)|p[3+2*i];return sum;}
int packet_public(const unsigned char *p,size_t n,unsigned *out){
 if(n<2||n>10||!p||!out)return 0;
 unsigned count=p[1];if(p[0]!=1||count>4||n!=2+2*(size_t)count)return 0;
 *out=unchecked(p,count);return 1;
}
static void change_length(size_t *n){*n=11;}
int main(void){unsigned char packet[10]={1,4,0,1,0,2,0,3,0,4};unsigned out=77;size_t n;
#ifdef __FRAMAC__
 n=(size_t)Frama_C_interval(0,10);
#else
 n=10;
#endif
 if(ANALYSIS_CASE==1)analysis_sink=unchecked(packet,5); /* Unchecked caller violates count<=4. */
 else if(ANALYSIS_CASE==2){if(n<=10){change_length(&n);analysis_sink=packet[n];}} /* Callback invalidates guard. */
 else if(ANALYSIS_CASE==3){if(n<=10){change_length(&n);if(n<10)analysis_sink=packet[n];}}
 else {analysis_sink=(unsigned)packet_public(packet,n,&out);analysis_sink=out;}
 return 0;
}
