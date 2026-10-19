#include "escape.h"
#ifndef ES_MUTANT
#define ES_MUTANT 0
#endif
static const unsigned char specials[5]={'&','<','>','"','\''};
static const char *const named[5]={"&amp;","&lt;","&gt;","&quot;","&apos;"};
static const size_t widths[5]={5,4,4,6,6};
static int admitted(unsigned char c) {return (c>=32&&c<=126)||c=='\n'||c=='\t';}
static size_t special(unsigned char c) {
 for(size_t i=0;i<5;i++)if(specials[i]==c)return i;
 return 5;
}
static int same(const unsigned char *s,size_t n,const char *t,size_t z) {
 if(n<z)return 0;
 for(size_t i=0;i<z;i++)if(s[i]!=(unsigned char)t[i])return 0;
 return 1;
}
static int decode(const unsigned char *s,size_t n,unsigned html,EsText *out) {
 EsText r={{0},0};
 for(size_t i=0;i<n;) {
  unsigned char c=s[i++];
  if(c=='&') {
   size_t k=0;
   for(;k<5;k++) {
    const char *word=html&&k==4?"&#39;":named[k];
    size_t z=html&&k==4?5:widths[k];
    if(same(s+i-1,n-(i-1),word,z)) {c=specials[k];i+=z-1;break;}
   }
   if(k==5)return ES_SYNTAX;
  } else if(!admitted(c)||special(c)!=5)return ES_SYNTAX;
  if(r.length==ES_PLAIN)return ES_RANGE;
  r.bytes[r.length++]=c;
 }
 *out=r;return ES_OK;
}
int es_encode(const unsigned char *s,size_t n,size_t cap,EsText *out) {
 if(!out||(!s&&n)||cap>ES_ENCODED)return ES_INVALID;
 if(n>ES_PLAIN)return ES_RANGE;
 EsText r={{0},0};
 for(size_t i=0;i<n;i++) {
  if(!admitted(s[i]))return ES_SYNTAX;
  size_t k=special(s[i]);
  if(ES_MUTANT==1&&s[i]=='&')k=5;
  if(k==5)r.bytes[r.length++]=s[i];
  else for(size_t j=0;j<widths[k];j++)r.bytes[r.length++]=(unsigned char)named[k][j];
 }
 if(r.length>cap)return ES_CAPACITY;
 *out=r;return ES_OK;
}
int es_decode(const unsigned char *s,size_t n,size_t cap,EsText *out) {
 if(!out||(!s&&n)||cap>ES_ENCODED)return ES_INVALID;
 if(n>ES_ENCODED)return ES_RANGE;
 EsText r={{0},0};int status=decode(s,n,0,&r);
 if(status)return status;
 if(ES_MUTANT==2) {
  EsText twice={{0},0};
  if(decode(r.bytes,r.length,0,&twice)==ES_OK)r=twice;
 }
 if(r.length>cap)return ES_CAPACITY;
 *out=r;return ES_OK;
}
int es_recode_html(const unsigned char *s,size_t n,size_t cap,EsText *out) {
 if(!out||(!s&&n)||cap>ES_ENCODED)return ES_INVALID;
 if(n>ES_ENCODED)return ES_RANGE;
 EsText plain={{0},0};int status=decode(s,n,1,&plain);
 return status?status:es_encode(plain.bytes,plain.length,cap,out);
}
