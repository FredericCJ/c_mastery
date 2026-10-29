/* The oracle stores a logical bit list and reconstructs integers by powers of2.
 * It does not duplicate the reference mask/shift formulae for transformations. */
#include "bits.h"
#include <stdio.h>
#define T(x) do{if(!(x)){fprintf(stderr,"FB_ORACLE_FAIL line=%d width=%u input=%u\n",__LINE__,width,input);return 1;}}while(0)
static unsigned power(unsigned n){unsigned p=1;while(n--)p*=2;return p;}
static unsigned value(const unsigned bits[8],unsigned width){unsigned v=0;for(unsigned i=0;i<width;++i)v+=bits[i]*power(i);return v;}
int main(void){
 unsigned long stats=0,shifts=0,fields=0,patches=0;
 for(unsigned width=1;width<=8;++width)for(unsigned input=0;input<256;++input){
  unsigned bits[8]={0},n=input,pop=0,bw=0;
  for(unsigned i=0;i<width;++i){bits[i]=n%2;n/=2;if(bits[i]){++pop;bw=i+1;}}
  struct fb_stats actual;T(fb_stats(width,input,&actual)==FB_OK&&actual.population==pop&&actual.width==bw&&actual.leading==width-bw);++stats;
  for(int count=-1;count<=(int)width;++count)for(unsigned policy=0;policy<3;++policy){
   fb_word out=917;
   if(count<0||count==(int)width){T(fb_shift(width,input,count,(enum fb_shift)policy,&out)==FB_RANGE&&out==917);}
   else {unsigned want[8]={0};for(unsigned i=0;i<width;++i){
     if(policy==FB_LEFT)want[i]=i>=(unsigned)count?bits[i-(unsigned)count]:0;
     else if(policy==FB_RIGHT)want[i]=i+(unsigned)count<width?bits[i+(unsigned)count]:0;
     else want[i]=bits[(i+width-(unsigned)count)%width];}
    T(fb_shift(width,input,count,(enum fb_shift)policy,&out)==FB_OK&&out==value(want,width));}
   ++shifts;
  }
  for(unsigned lo=0;lo<=width;++lo)for(unsigned hi=lo;hi<=width;++hi){
   fb_word out=917;unsigned want[8]={0};for(unsigned i=lo;i<hi;++i)want[i-lo]=bits[i];
   T(fb_extract(width,input,lo,hi,&out)==FB_OK&&out==value(want,hi-lo));
   /* all represented field values plus the first rejected value */
   for(unsigned v=0;v<=power(hi-lo);++v){out=917;
    if(v==power(hi-lo)){T(fb_insert(width,input,(struct fb_edit){lo,hi,v},&out)==FB_RANGE&&out==917);}
    else{for(unsigned i=0;i<width;++i)want[i]=bits[i];unsigned nv=v;
     for(unsigned i=lo;i<hi;++i){want[i]=nv%2;nv/=2;}
     T(fb_insert(width,input,(struct fb_edit){lo,hi,v},&out)==FB_OK&&out==value(want,width));}++fields;
   }
  }
 }
 /* Two independently specified edits of a4-bit word. Per-bit disagreement is
  * the simultaneous conflict oracle; sequential order uses last writer. */
 unsigned width=4;
 for(unsigned input=0;input<16;++input)for(unsigned lo=0;lo<=4;++lo)for(unsigned hi=lo;hi<=4;++hi)
 for(unsigned l2=0;l2<=4;++l2)for(unsigned h2=l2;h2<=4;++h2)
 for(unsigned av=0;av<power(hi-lo);++av)for(unsigned bv=0;bv<power(h2-l2);++bv){
  unsigned bits[8]={0},written[8]={0},first[8]={0},n=input,na=av,nb=bv,conflict=0;
  for(unsigned i=0;i<4;++i){bits[i]=n%2;n/=2;}
  for(unsigned i=lo;i<hi;++i){bits[i]=na%2;na/=2;written[i]=1;first[i]=bits[i];}
  for(unsigned i=l2;i<h2;++i){unsigned bit=nb%2;nb/=2;if(written[i]&&first[i]!=bit)conflict=1;bits[i]=bit;}
  fb_word out=917;T(fb_patch(4,input,(struct fb_edit){lo,hi,av},(struct fb_edit){l2,h2,bv},FB_SEQUENTIAL,&out)==FB_OK&&out==value(bits,4));
  out=917;T(fb_patch(4,input,(struct fb_edit){lo,hi,av},(struct fb_edit){l2,h2,bv},FB_SIMULTANEOUS,&out)==(conflict?FB_CONFLICT:FB_OK)&&out==(conflict?917:value(bits,4)));++patches;
 }
 printf("FB_ORACLE_PASS stats=%lu shifts=%lu fields=%lu patches=%lu\n",stats,shifts,fields,patches);return 0;
}
