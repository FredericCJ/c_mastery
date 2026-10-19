#include "algorithms.h"
#include "free.h"
#ifndef AD_MUTANT
#define AD_MUTANT 0
#endif
static int span(const void*p,size_t n){return n<=64&&(!n||p);}
enum ADStatus ad_byte_copy(unsigned char*out,size_t capacity,const unsigned char*p,size_t n){if(!span(p,n)||!span(out,capacity))return AD_INVALID;if(capacity<n)return AD_SPACE;fr_copy(out,p,n);return AD_OK;}
enum ADStatus ad_byte_move(unsigned char*p,size_t n,size_t destination,size_t source,size_t count){if(!span(p,n))return AD_INVALID;enum fr_status rc=fr_arena_move(p,n,destination,source,count);return rc==FR_OK?AD_OK:AD_INVALID;}
enum ADStatus ad_byte_compare(const unsigned char*a,size_t n,const unsigned char*b,size_t m,int*out){if(!span(a,n)||!span(b,m)||!out)return AD_INVALID;size_t common=n<m?n:m;int relation=fr_compare(a,b,common);*out=relation?relation:n<m?-1:n>m;return AD_OK;}
enum ADStatus ad_byte_find(const unsigned char*p,size_t n,unsigned char value,size_t*out){if(!span(p,n)||!out)return AD_INVALID;size_t i=0;while(i<n&&p[i]!=value)++i;*out=i;return AD_OK;}
enum ADStatus ad_byte_split(const unsigned char*p,size_t n,unsigned char delimiter,unsigned keep_empty,ADSpan*out,size_t capacity,size_t*count){
 if(!span(p,n)||keep_empty>1||!count)return AD_INVALID;
 ADSpan parts[65];size_t used=0,start=0;for(size_t i=0;i<=n;++i){if(i==n||p[i]==delimiter){if(keep_empty||i>start)parts[used++]=(ADSpan){start,i-start};start=i+1;}}
 if(capacity<used||(used&&!out)){return AD_SPACE;}for(size_t i=0;i<used;++i)out[i]=parts[i];*count=used;return AD_OK;
}
static int whitespace(unsigned char c){return c==32||c==9||c==10||c==13||c==11||c==12;}
enum ADStatus ad_byte_trim(const unsigned char*p,size_t n,ADSpan*out){if(!span(p,n)||!out)return AD_INVALID;size_t start=0,end=n;while(start<end&&whitespace(p[start]))++start;while(end>start&&whitespace(p[end-1]))--end;*out=(ADSpan){start,end-start};return AD_OK;}
enum ADStatus ad_byte_trim_in_place(unsigned char*p,size_t n,size_t*out){if(!out)return AD_INVALID;ADSpan part;enum ADStatus rc=ad_byte_trim(p,n,&part);if(rc!=AD_OK)return rc;fr_arena_move(p,n,0,part.offset,part.length);*out=part.length;return AD_OK;}
static unsigned digit(unsigned char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;if(c>='A'&&c<='F')return c-'A'+10;return 16;}
static enum ADStatus parse(const unsigned char*p,size_t n,unsigned base,unsigned prefix,unsigned whole,unsigned is_signed,uint32_t*magnitude,unsigned*negative,size_t*consumed){
 if(!span(p,n)||base<2||base>16||prefix>1||whole>1||!n)return AD_INVALID;
 size_t i=0;unsigned sign=0;if(p[i]=='+'||p[i]=='-'){sign=p[i]=='-';if(sign&&!is_signed)return AD_INVALID;++i;}
 if(prefix&&base==16&&n-i>=2&&p[i]=='0'&&(p[i+1]=='x'||p[i+1]=='X'))i+=2;
 size_t start=i;uint32_t value=0,limit=is_signed?(sign?UINT32_C(2147483648):INT32_MAX):UINT32_MAX;
 while(i<n){unsigned d=digit(p[i]);if(d>=base)break;if(value>(limit-d)/base)return AD_OVERFLOW;value=value*base+d;++i;}
 if(i==start||(whole&&i!=n&&AD_MUTANT!=6)){return AD_INVALID;}*magnitude=value;*negative=sign;*consumed=i;return AD_OK;
}
enum ADStatus ad_parse_i32(const unsigned char*p,size_t n,unsigned base,unsigned prefix,unsigned whole,int32_t*out,size_t*consumed){if(!out||!consumed)return AD_INVALID;uint32_t magnitude;unsigned negative;size_t used;enum ADStatus rc=parse(p,n,base,prefix,whole,1,&magnitude,&negative,&used);if(rc!=AD_OK)return rc;*out=negative?(magnitude==UINT32_C(2147483648)?INT32_MIN:-(int32_t)magnitude):(int32_t)magnitude;*consumed=used;return AD_OK;}
enum ADStatus ad_parse_u32(const unsigned char*p,size_t n,unsigned base,unsigned prefix,unsigned whole,uint32_t*out,size_t*consumed){if(!out||!consumed)return AD_INVALID;uint32_t magnitude;unsigned negative;size_t used;enum ADStatus rc=parse(p,n,base,prefix,whole,0,&magnitude,&negative,&used);if(rc!=AD_OK)return rc;*out=magnitude;*consumed=used;return AD_OK;}
enum ADStatus ad_format_i32(int32_t value,unsigned base,unsigned char*out,size_t capacity,size_t*count){
 if(base<2||base>16||!count)return AD_INVALID;
 unsigned char reverse[33];size_t n=0;uint32_t magnitude=value<0?(uint32_t)(-(value+1))+1u:(uint32_t)value;
 do{unsigned d=magnitude%base;reverse[n++]=(unsigned char)(d<10?'0'+d:'a'+d-10);magnitude/=base;}while(magnitude);if(value<0)reverse[n++]='-';
 if(capacity<n||!out){return AD_SPACE;}for(size_t i=0;i<n;++i)out[i]=reverse[n-i-1];*count=n;return AD_OK;
}
