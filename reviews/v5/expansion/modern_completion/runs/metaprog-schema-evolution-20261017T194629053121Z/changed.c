#include "schema.h"
#include <stdio.h>
#include <string.h>
static unsigned calls,tags;
static int see(void *p,const MpsField *f,int64_t v){(void)p;++calls;tags=tags*10+f->tag;return f->tag!=4||v==15;}
static int old_equal(MpsRecord a,MpsRecord b){return a.present==b.present&&a.code==b.code&&a.value==b.value&&a.note==b.note;}
#define C(x) do{if(!(x)){fprintf(stderr,"MPS_EVOLUTION %d %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){MpsRecord a={15,0x1234,0x89abcdef,-2,15},b=a,sentinel={3,99,88,77,6};unsigned char wire[64],old[64];const MpsField *f;size_t n=999,visited=999;unsigned equal=99;
unsigned char expected[]={3,4,1,2,0x12,0x34,2,4,0x89,0xab,0xcd,0xef,3,5,0xff,0xff,0xff,0xfe,1,4,2,0,15};
C(mps_fields(&f)==4&&f[3].offset==offsetof(MpsRecord,quality)&&f[3].native_size==2&&f[3].width==2&&!strcmp(f[3].name,"quality"));
C(mps_validate(&a)==MPS_OK&&mps_encode(3,&a,wire,64,&n)==MPS_OK&&n==23&&!memcmp(wire,expected,23));
C(mps_decode(wire,n,&b)==MPS_OK&&b.quality==15);b.quality=14;C(old_equal(a,b));C(mps_equal(&a,&b,&equal)==MPS_OK&&!equal);
C(mps_visit(&a,see,NULL,&visited)==MPS_OK&&visited==4&&calls==4&&tags==1234);
memcpy(old,wire,64);n=999;C(mps_encode(2,&a,wire,64,&n)==MPS_RANGE&&n==999&&!memcmp(old,wire,64));
a.quality=16;C(mps_validate(&a)==MPS_RANGE&&mps_encode(3,&a,wire,64,&n)==MPS_RANGE&&n==999&&!memcmp(old,wire,64));
wire[22]=16;b=sentinel;C(mps_decode(wire,23,&b)==MPS_RANGE&&b.present==sentinel.present&&b.quality==sentinel.quality&&b.code==sentinel.code);
a.present=7;C(mps_validate(&a)==MPS_OK&&mps_encode(2,&a,wire,64,&n)==MPS_OK&&n==19);C(mps_decode(wire,n,&b)==MPS_OK&&b.quality==0&&b.present==7);
C(mps_encode(3,&b,wire,64,&n)==MPS_OK&&wire[0]==3&&n==19);MPS_LINK_GUARD();
printf("{\"record_size\":%zu,\"offsets\":[%zu,%zu,%zu,%zu],\"wire\":\"030401021234020489abcdef0305fffffffe010402000f\",\"stale_consumer_compiled\":true,\"quality_limit\":15,\"visited\":4}\n",sizeof a,f[0].offset,f[1].offset,f[2].offset,f[3].offset);return 0;}
