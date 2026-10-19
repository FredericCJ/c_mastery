#include "http_depth.h"
#include <limits.h>
_Static_assert(sizeof(size_t)==2 && SIZE_MAX==65535u,"This campaign requires16-bit size_t");
_Static_assert(sizeof(uint32_t)==4 && UINT32_MAX==4294967295UL,"Wire numeral remainsu32");
/* ATmega328P USART0, exercised only by the named QEMU model. No libc calls. */
static void put(unsigned char c){while(!(*(volatile unsigned char *)0xc0 & 0x20)){}*(volatile unsigned char *)0xc6=c;}
static void report(int pass){put('A');put('V');put('R');put(':');put(pass?'P':'F');put('A');put(pass?'S':'I');put(pass?'S':'L');put('\n');}
int main(void) {
  *(volatile unsigned char *)0xc1=0x08; /* TXEN0 */
  volatile unsigned char seed=0;unsigned char s[10];uint32_t out=77;int pass=1;
  s[0]=(unsigned char)('1'+seed);s[1]='0';s[2]='2';s[3]='5';
  pass &= hd_length(s,4,1024,&out)==413 && out==77;
  s[4]='x';pass &= hd_length(s,5,1024,&out)==400 && out==77;
  s[0]='4';s[1]='2';s[2]='9';s[3]='4';s[4]='9';s[5]='6';s[6]='7';s[7]='2';s[8]='9';s[9]='5';
  pass &= hd_length(s,10,1024,&out)==413 && out==77;
  s[9]='6';pass &= hd_length(s,10,1024,&out)==400 && out==77;
  s[0]='1';s[1]='0';s[2]='2';s[3]='4';pass &= hd_length(s,4,1024,&out)==0 && out==1024;
  size_t admitted=(size_t)out;pass &= admitted==1024;report(pass);
  for(;;)__asm__ volatile("sleep");
}
