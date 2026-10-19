#include "bits.h"
#include "integer.h" /* Actual prerequisite cell codec. */
#include <limits.h>
#ifndef FB_MUTANT
#define FB_MUTANT 0
#endif
static int width_ok(unsigned width){return width>0 && width<=16;}
static FbWord mask(unsigned width){return ((FbWord)1<<width)-1;}
FbStatus fb_shift(unsigned width,FbWord input,int count,FbShift policy,FbWord *out){
 if(!out || !width_ok(width) || (unsigned)policy>FB_ROTATE_LEFT)return FB_INVALID;
 if(count<0 || (unsigned)count>=width)return FB_RANGE;
 FbWord value=FB_MUTANT==1?input:input&mask(width),result;
 if(policy==FB_LEFT)result=value<<count;
 else if(policy==FB_RIGHT)result=value>>count;
 else result=count?(value<<count)|(value>>(width-(unsigned)count)):value;
 *out=result&mask(width);return FB_OK;
}
FbStatus fb_extract(unsigned width,FbWord input,unsigned low,unsigned high,FbWord *out){
 if(!out || !width_ok(width))return FB_INVALID;
 if(low>high || high>width)return FB_RANGE;
 *out=(input>>low)&mask(high-low);return FB_OK;
}
FbStatus fb_insert(unsigned width,FbWord input,FbEdit edit,FbWord *out){
 if(!out || !width_ok(width))return FB_INVALID;
 if(edit.low>edit.high || edit.high>width)return FB_RANGE;
 FbWord field=mask(edit.high-edit.low);
 if(edit.value>field)return FB_RANGE;
 *out=((input&~(field<<edit.low))|(edit.value<<edit.low))&mask(width);return FB_OK;
}
FbStatus fb_patch(unsigned width,FbWord input,FbEdit a,FbEdit b,FbPatch policy,FbWord *out){
 if(!out || (unsigned)policy>FB_SIMULTANEOUS)return FB_INVALID;
 FbWord first,second;FbStatus s=fb_insert(width,input,a,&first);if(s!=FB_OK)return s;
 s=fb_insert(width,first,b,&second);if(s!=FB_OK)return s;
 if(policy==FB_SIMULTANEOUS && FB_MUTANT!=2){
  FbWord overlap=(mask(a.high-a.low)<<a.low)&(mask(b.high-b.low)<<b.low);
  if(((a.value<<a.low)^(b.value<<b.low))&overlap)return FB_CONFLICT;
 }
 *out=second;return FB_OK;
}
FbStatus fb_stats(unsigned width,FbWord input,FbStats *out){
 if(!out || !width_ok(width))return FB_INVALID;
 FbWord value=input&mask(width);FbStats s={0,0,width};
 for(unsigned i=0;i<width;++i)if((value>>i)&1){++s.population;s.width=i+1;}
 s.leading=width-s.width;if(FB_MUTANT==3 && !value)s.leading=0;
 *out=s;return FB_OK;
}
FbStatus fb_encode(FbWord value,FbOrder order,unsigned char *out,size_t capacity){
 if(!out || (unsigned)order>FB_LITTLE)return FB_INVALID;
 if(CHAR_BIT!=8)return FB_UNSUPPORTED;
 if(capacity<4)return FB_SHORT;
 unsigned int cells[4];if(fi_encode_cells(value,cells)!=FI_OK)return FB_RANGE;
 for(unsigned i=0;i<4;++i)out[i]=(unsigned char)cells[order==FB_BIG?i:3-i];
 return FB_OK;
}
static int span_ok(FbSpan s){return s.data || !s.size;}
FbStatus fb_decode_two(FbSpan a,FbSpan b,FbOrder order,FbWord *out){
 if(!out || !span_ok(a) || !span_ok(b) || (unsigned)order>FB_LITTLE)return FB_INVALID;
 if(CHAR_BIT!=8)return FB_UNSUPPORTED;
 if(FB_MUTANT==4)*out=0;
 if(a.size<4 && b.size<4-a.size)return FB_SHORT;
 unsigned int cells[4];
 for(unsigned i=0;i<4;++i){unsigned char v=i<a.size?a.data[i]:b.data[i-a.size];cells[order==FB_BIG?i:3-i]=v;}
 FbWord value;if(fi_decode_cells(cells,&value)!=FI_OK)return FB_RANGE;*out=value;return FB_OK;
}
FbStatus fb_decode(FbSpan input,FbOrder order,FbWord *out){return fb_decode_two(input,(FbSpan){NULL,0},order,out);}
static unsigned u16(const unsigned char *p){return (unsigned)p[0]*256u+p[1];}
FbStatus fb_record(FbSpan input,FbUnknown policy,FbRecord *out){
 if(!out || !span_ok(input) || (unsigned)policy>FB_SKIP_UNKNOWN)return FB_INVALID;
 if(CHAR_BIT!=8)return FB_UNSUPPORTED;
 if(input.size<4)return FB_SHORT;
 unsigned version=input.data[0],flags=input.data[1],size=u16(input.data+2);
 if(size<4)return FB_RANGE;
 if(size>input.size)return FB_SHORT;
 if(version!=1 || (flags&~1u)){
  if(policy==FB_REJECT_UNKNOWN)return FB_UNSUPPORTED;
  *out=(FbRecord){0,0,size};return FB_SKIPPED;
 }
 if(size!=8)return FB_RANGE;
 FbWord value;FbStatus s=fb_decode((FbSpan){input.data+4,4},FB_BIG,&value);
 if(s!=FB_OK)return s;
 *out=(FbRecord){value,flags,size};return FB_OK;
}
FbStatus fb_negotiate(FbSpan input,FbPlugin *out){
 if(!out || !span_ok(input))return FB_INVALID;
 if(CHAR_BIT!=8)return FB_UNSUPPORTED;
 if(input.size<4)return FB_SHORT;
 unsigned version=u16(input.data),size=u16(input.data+2);
 if(size<4)return FB_RANGE;
 if(size>input.size)return FB_SHORT;
 if(version!=1 && version!=2)return FB_UNSUPPORTED;
 if(size<(version==1?8u:12u))return FB_SHORT;
 FbWord features=0,items=0;FbStatus s=fb_decode((FbSpan){input.data+4,4},FB_BIG,&features);
 if(s!=FB_OK)return s;
 if(features&~(FbWord)1)return FB_UNSUPPORTED;
 if(version==2){s=fb_decode((FbSpan){input.data+8,4},FB_BIG,&items);if(s!=FB_OK)return s;}
 if(FB_MUTANT==6 && version==1)items=999;
 *out=(FbPlugin){version,size,features,items};return FB_OK;
}
FbStatus fb_register_init(FbRegister *r,unsigned value,unsigned read_clear){
 if(!r || value>255 || read_clear>1)return FB_INVALID;
 *r=(FbRegister){(unsigned char)value,0,0,read_clear};return FB_OK;
}
unsigned fb_register_read(FbRegister *r){unsigned value=r->state;++r->reads;if(r->read_clear)r->state&=15;return value;}
void fb_register_write(FbRegister *r,unsigned v){++r->writes;r->state=(unsigned char)((v&15)|((r->state&240)&~(v&240)));}
FbStatus fb_register_update(FbRegister *r,unsigned low,unsigned clear){
 if(!r || low>15 || clear>240 || (clear&15))return FB_INVALID;
 if(FB_MUTANT==5){unsigned value=fb_register_read(r);fb_register_write(r,(value&240)|low|clear);}
 else fb_register_write(r,low|clear);
 return FB_OK;
}
