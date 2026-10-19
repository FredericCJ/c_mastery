/* Intentionally incorrect shared authorities. Do not repair these controls. */
#include <stdint.h>
#include <stdio.h>
#include <stddef.h>
static uint16_t wrong_endian(const unsigned char *p){return (uint16_t)(p[1]*256u+p[0]);}
static size_t wrong_length(unsigned count){return 2*(size_t)count;}
static void cursor(const unsigned char *p,uint16_t out[2]){for(unsigned i=0;i<2;i++)out[i]=wrong_endian(p+2+2*i);}
static void fold(const unsigned char *p,uint16_t out[2]){unsigned pair=0;unsigned char bytes[2];for(unsigned i=2;i<6;i++){bytes[(i-2)%2]=p[i];if(i&1u)out[pair++]=wrong_endian(bytes);}}
int main(void){
 const unsigned char wire[6]={1,2,1,2,254,220};uint16_t a[2],b[2];cursor(wire,a);fold(wire,b);
 int length_a=sizeof wire==wrong_length(wire[1]);
 size_t count=wire[1];int length_b=wrong_length((unsigned)count)==sizeof wire;
 printf("CORRELATED %u %u %u %u %d %d\n",a[0],a[1],b[0],b[1],length_a,length_b);
 return 0;
}
