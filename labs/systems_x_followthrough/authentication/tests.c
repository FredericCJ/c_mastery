#include "auth.h"
#include "hashing.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"AUTH_X_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
typedef struct {unsigned char key[32];unsigned calls,fail;} Key;
static int keyed(void*v,const unsigned char*m,size_t n,unsigned char tag[32]){Key*k=v;k->calls++;if(k->fail)return B5_CONFLICT;return rph_hmac(k->key,32,m,n,tag)==RPH_OK?B5_OK:B5_CONFLICT;}
static int seal(Key*k,uint64_t id,SxAuthenticated*r){unsigned char m[32];if(sx_auth_encode(id,&r->record,m)!=B5_OK)return 0;return keyed(k,m,32,r->tag)==B5_OK;}
int main(void){Key key={{1,2,3,4},0,0};SxAuthenticated r[3]={0},bad[3];uint32_t out=999;unsigned char m[32];for(unsigned i=0;i<2;i++){r[i].record=(B5Redo){i+1,10+i,b5_checksum(i+1,10+i),1};CHECK(seal(&key,7,&r[i]));}CHECK(sx_auth_recover(7,0,0,r,3,keyed,&key,&out)==B5_OK&&out==11);
 for(unsigned field=0;field<4;field++){memcpy(bad,r,sizeof r);if(field==0){bad[0].record.value^=1;bad[0].record.checksum^=1;}else bad[0].tag[(field-1)*15]^=1;out=999;CHECK(sx_auth_recover(7,0,0,bad,3,keyed,&key,&out)==B5_CONFLICT&&out==999);}
 out=999;CHECK(sx_auth_recover(8,0,0,r,3,keyed,&key,&out)==B5_CONFLICT&&out==999);key.fail=1;CHECK(sx_auth_recover(7,0,0,r,3,keyed,&key,&out)==B5_CONFLICT&&out==999);key.fail=0;key.key[0]^=1;CHECK(sx_auth_recover(7,0,0,r,3,keyed,&key,&out)==B5_CONFLICT&&out==999);key.key[0]^=1;
 CHECK(sx_auth_recover(7,2,11,r,3,keyed,&key,&out)==B5_OK&&out==11);r[1].record.generation=3;r[1].record.checksum=b5_checksum(3,11);CHECK(seal(&key,7,&r[1]));out=999;CHECK(sx_auth_recover(7,0,0,r,3,keyed,&key,&out)==B5_CONFLICT&&out==999);
 r[0].record=(B5Redo){1,10,b5_checksum(1,10),1};CHECK(sx_auth_encode(7,&r[0].record,m)==B5_OK);for(unsigned i=0;i<32;i++)printf("%02x",m[i]);puts("");CHECK(keyed(&key,m,32,r[0].tag)==B5_OK);for(unsigned i=0;i<32;i++)printf("%02x",r[0].tag[i]);puts("");rph_clear(&key,sizeof key);puts("AUTH_X_PASS tamper_key_domain_generation_tail unchanged_output");return 0;}
