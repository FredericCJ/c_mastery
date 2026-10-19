#include "hex.h"
enum SfStatus sf_window(const struct SfImage *image,uint64_t begin,uint64_t end)
{
 if(!image||image->count>64||begin>=end||end>UINT64_C(4294967296))return SF_INVALID;
 for(size_t i=0;i<image->count;i++){
  if(image->bytes[i].address<begin||image->bytes[i].address>=end)return SF_RANGE;
#ifdef SF_MUT_WINDOW
  break;
#endif
 }
 return SF_OK;
}
static int digit(unsigned char c)
{
 if(c>='0'&&c<='9')return c-'0';
 if(c>='A'&&c<='F')return c-'A'+10;
 if(c>='a'&&c<='f')return c-'a'+10;
 return -1;
}
enum SfStatus sf_hex(const char *text,size_t length,enum SfMode mode,struct SfImage *out)
{
 struct SfImage tmp={0};size_t pos=0,records=0;uint32_t base=0;int eof=0;
 if(!text||!out||(unsigned)mode>SF_LINEAR||length>4096)return SF_INVALID;
 while(pos<length){
  unsigned char raw[260];size_t n=0;unsigned sum=0;
  if(eof)return SF_FORMAT;
  if(++records>16)return SF_CAPACITY;
  if(text[pos++]!=':')return SF_FORMAT;
  while(pos<length&&text[pos]!='\r'&&text[pos]!='\n'){
   int a,b;if(length-pos<2||n==sizeof raw)return SF_FORMAT;
   a=digit((unsigned char)text[pos]);b=digit((unsigned char)text[pos+1]);
   if(a<0||b<0)return SF_FORMAT;
   raw[n]=(unsigned char)(a*16+b);sum+=raw[n++];pos+=2;
  }
  if(pos<length&&text[pos]=='\r'){pos++;if(pos==length||text[pos]!='\n')return SF_FORMAT;}
  if(pos<length&&text[pos]=='\n')pos++;
  if(n<5||n!=(size_t)raw[0]+5)return SF_FORMAT;
#ifndef SF_MUT_CHECKSUM
  if(sum%256)return SF_CHECKSUM;
#else
  (void)sum;
#endif
  unsigned kind=raw[3],count=raw[0];uint32_t offset=(uint32_t)raw[1]*256+raw[2];
  if(kind==0){
   if(count>64-tmp.count)return SF_CAPACITY;
   if(mode==SF_PLAIN&&count>65536-offset)return SF_UNSUPPORTED;
   for(unsigned j=0;j<count;j++){
    uint32_t address;
    if(mode==SF_SEGMENT){
#ifdef SF_MUT_SEGMENT
     address=base+offset+j;
#else
     address=base+((offset+j)&UINT32_C(65535));
#endif
    }else address=(uint32_t)(base+offset+j);
    for(size_t k=0;k<tmp.count;k++)if(tmp.bytes[k].address==address)return SF_DUPLICATE;
    tmp.bytes[tmp.count].address=address;tmp.bytes[tmp.count++].value=raw[4+j];
   }
  }else if(kind==1){if(count||offset)return SF_FORMAT;eof=1;
  }else if(kind==2||kind==4){
   if(count!=2||offset)return SF_FORMAT;
   if((kind==2&&mode!=SF_SEGMENT)||(kind==4&&mode!=SF_LINEAR))return SF_UNSUPPORTED;
   base=((uint32_t)raw[4]*256+raw[5])<<(kind==2?4:16);
  }else if(kind==3||kind==5){
   if(count!=4||offset)return SF_FORMAT;
   if(mode==SF_PLAIN||(kind==5&&mode!=SF_LINEAR))return SF_UNSUPPORTED;
   if(tmp.entry_kind)return SF_DUPLICATE;
   tmp.entry_kind=kind;tmp.entry=0;
   for(unsigned j=0;j<4;j++)tmp.entry=(tmp.entry<<8)|raw[4+j];
#ifdef SF_MUT_ENTRY
   if(tmp.count)tmp.bytes[0].address=tmp.entry;
#endif
  }else return SF_UNSUPPORTED;
 }
 if(!eof)return SF_FORMAT;
 *out=tmp;return SF_OK;
}
