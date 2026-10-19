#include "schema.h"
#include "generated/config.h"
#include "generated/fields.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"LP_CHECK:%s\n",#x);return 1;}}while(0)
int main(int argc,char **argv)
{
 if(argc!=2)return 2;
 if(!strcmp(argv[1],"wire")){LsFields f={5,17,2},g;uint16_t w;CHECK(ls_pack(&f,&w)==0 && w==653);CHECK(ls_unpack(w,&g)==0 && g.tag==5 && g.length==17 && g.flags==2);CHECK(LS_TAG_MASK==7 && LS_LENGTH_MASK==248 && LS_FLAGS_MASK==768);puts("LP wire validated");}
 else if(!strcmp(argv[1],"identity")){CHECK(ls_require(LS_WIRE_ID,LS_ABI_ID)==0);CHECK(ls_require("incompatible",LS_ABI_ID)==-1);CHECK(ls_require(LS_WIRE_ID,"incompatible")==-1);CHECK(ls_record_size()==12);puts("LP identity validated");}
 else if(!strcmp(argv[1],"boundary")){LsFields f={8,0,0},g={91,92,93};uint16_t w=77;CHECK(ls_pack(&f,&w)==-1 && w==77);CHECK(ls_unpack(1024,&g)==-1 && g.tag==91 && g.length==92 && g.flags==93);f.tag=7;f.length=31;f.flags=3;CHECK(ls_pack(&f,&w)==0 && w==1023);puts("LP boundary validated");}
 else if(!strcmp(argv[1],"oracle")){for(uint32_t a=0;a<8;++a)for(uint32_t b=0;b<32;++b)for(uint32_t c=0;c<4;++c){LsFields f={a,b,c},g;uint16_t w;CHECK(ls_pack(&f,&w)==0);CHECK(ls_unpack(w,&g)==0);printf("%u %u %u %u %u %u %u\n",a,b,c,(unsigned)w,g.tag,g.length,g.flags);}}
 else if(!strcmp(argv[1],"metrics"))printf("{\"record_bytes\":%zu,\"fields_bytes\":%zu,\"allocations\":0,\"carrier_bits\":16}\n",ls_record_size(),sizeof(LsFields));
 else return 2;
 return 0;
}
