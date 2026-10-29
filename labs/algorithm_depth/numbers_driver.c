/* Hosted verification adapter. scanf/printf are not target dependencies. */
#include "algorithms.h"
#include <stdio.h>
#include <stdint.h>
#include <string.h>
static unsigned nibble(char c){return c<='9'?(unsigned)(c-'0'):(unsigned)(c-'a'+10);}
int main(void){char mode;while(scanf(" %c",&mode)==1){if(mode=='F'){long long input;unsigned base;if(scanf("%lld %u",&input,&base)!=2||input<INT32_MIN||input>INT32_MAX)return 2;unsigned char text[64];size_t n=0;enum ad_status rc=ad_format_i32((int32_t)input,base,text,sizeof text,&n);printf("%u %zu ",(unsigned)rc,n);for(size_t i=0;i<n;++i)putchar(text[i]);putchar('\n');}
 else if(mode=='I'||mode=='U'){unsigned base,prefix,whole;size_t n;char hex[129];if(scanf("%u %u %u %zu %128s",&base,&prefix,&whole,&n,hex)!=5||n>64||(n&&strlen(hex)!=2*n))return 2;unsigned char input[64];for(size_t i=0;i<n;++i)input[i]=(unsigned char)(16*nibble(hex[2*i])+nibble(hex[2*i+1]));size_t used=88;enum ad_status rc;if(mode=='I'){int32_t value=77;rc=ad_parse_i32(input,n,base,prefix,whole,&value,&used);printf("%u %lld %zu\n",(unsigned)rc,(long long)value,used);}else{uint32_t value=77;rc=ad_parse_u32(input,n,base,prefix,whole,&value,&used);printf("%u %llu %zu\n",(unsigned)rc,(unsigned long long)value,used);}}
 else return 2;}return 0;}
