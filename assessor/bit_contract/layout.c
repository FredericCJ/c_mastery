#include "bits.h"
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#if FB_PACK
#pragma pack(push,1)
#endif
struct Fields {unsigned int a:3,b:5,c:9;};
struct WideFields {unsigned long a:3,b:5,c:9;}; /* documented compiler extension */
struct Record {unsigned char tag;uint32_t value;unsigned short count;};
struct Reordered {unsigned short count;uint32_t value;unsigned char tag;};
#if FB_PACK
#pragma pack(pop)
#endif
struct __attribute__((packed)) Packed {unsigned char tag;uint32_t value;unsigned short count;};
static void dump(const char *name,const void *object,size_t size,size_t alignment){
 const unsigned char *p=object;printf("LAYOUT %s size=%zu align=%zu bytes=",name,size,alignment);
 for(size_t i=0;i<size;++i)printf("%02x",p[i]);
 putchar('\n');
}
int main(void){
 _Static_assert(CHAR_BIT==8 && sizeof(uint32_t)==4 && sizeof(unsigned short)==2,"recorded native layout model");
 struct Fields a;struct WideFields b;struct Record r;struct Reordered reordered;struct Packed packed;
 memset(&a,0,sizeof a);memset(&b,0,sizeof b);memset(&r,0,sizeof r);memset(&reordered,0,sizeof reordered);memset(&packed,0,sizeof packed);
 a.a=5;a.b=17;a.c=257;b.a=5;b.b=17;b.c=257;
 r.tag=reordered.tag=packed.tag=1;r.value=reordered.value=packed.value=UINT32_C(0x12345678);r.count=reordered.count=packed.count=2;
 printf("ABI char_bits=%d pointer=%zu uint=%zu ulong=%zu pack=%d edition=%ld\n",CHAR_BIT,sizeof(void*),sizeof(unsigned),sizeof(unsigned long),FB_PACK,(long)__STDC_VERSION__);
 dump("fields359",&a,sizeof a,_Alignof(struct Fields));dump("wide359",&b,sizeof b,_Alignof(struct WideFields));
 dump("record",&r,sizeof r,_Alignof(struct Record));dump("reordered",&reordered,sizeof reordered,_Alignof(struct Reordered));dump("packed",&packed,sizeof packed,_Alignof(struct Packed));
 printf("OFFSETS record=%zu,%zu,%zu reordered=%zu,%zu,%zu packed=%zu,%zu,%zu\n",offsetof(struct Record,tag),offsetof(struct Record,value),offsetof(struct Record,count),offsetof(struct Reordered,tag),offsetof(struct Reordered,value),offsetof(struct Reordered,count),offsetof(struct Packed,tag),offsetof(struct Packed,value),offsetof(struct Packed,count));
 unsigned char be[4],le[4];FbWord decoded;
 if(fb_encode(r.value,FB_BIG,be,4)!=FB_OK||fb_encode(r.value,FB_LITTLE,le,4)!=FB_OK||be[0]!=18||le[0]!=120)return 1;
 if(fb_decode((FbSpan){be,4},FB_BIG,&decoded)!=FB_OK||decoded!=r.value)return 1;
 /* A declared packed object avoids forging a typed lvalue into a char array.
  * Its host-endian interpretation of copied wire bytes still differs. */
 const unsigned char wire[]={1,18,52,86,120,0,2};memcpy(&packed,wire,sizeof packed);
 printf("PACKED_WIRE_OBS value=%ju count=%u portable_decoder=%ju\n",(uintmax_t)packed.value,(unsigned)packed.count,(uintmax_t)decoded);
 printf("FB_LAYOUT_PASS values=%u,%u,%u no_padding_goldens no_bitfield_address\n",a.a,a.b,a.c);return 0;
}
