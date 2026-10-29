#include "lookup.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"SL_CHECK line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static uint8_t disk[67][4096];
static unsigned reads;
static int rd(void *c,uint32_t p,void *b){(void)c;if(p>=67)return 1;reads++;memcpy(b,disk[p],4096);return 0;}
static int wr(void *c,uint32_t p,const void *b){(void)c;if(p>=67)return 1;memcpy(disk[p],b,4096);return 0;}
static int fl(void *c){(void)c;return 0;}
static int open_empty(store *s){store_io io={NULL,rd,wr,fl,67};memset(disk,0,sizeof disk);return store_format(io)||store_open(s,io)||store_begin(s);}
static size_t key(unsigned i,uint8_t out[4]){if(i<2){out[0]=0;out[1]=0;return i+1;}static const uint8_t first[]={0,1,128,255};i-=2;out[0]=first[i/23];out[1]=0;out[2]=(uint8_t)(i%23);return 3;}
static int setup(store *s){if(open_empty(s))return 1;for(unsigned j=0;j<92;j++){unsigned i=(j*37)%92;uint8_t k[4],v=(uint8_t)i;size_t n=key(i,k);if(store_put(s,k,n,&v,1)||store_check(s))return 1;}return 0;}
struct store_collected {struct sl_record records[SL_RECORDS];size_t count,limit,calls;};
static int collect(void *p,const uint8_t *k,size_t n,const uint8_t *v,size_t z){struct store_collected *c=p;c->calls++;if(c->count==c->limit)return 1;struct sl_record *r=&c->records[c->count++];r->klen=(uint8_t)n;r->vlen=(uint8_t)z;memcpy(r->key,k,n);memcpy(r->value,v,z);return c->count==c->limit;}
static int compare(const uint8_t *a,size_t n,const uint8_t *b,size_t m){size_t z=n<m?n:m;int c=memcmp(a,b,z);return c?c:n<m?-1:n>m?1:0;}
static int binary(void){
 store s;CHECK(setup(&s)==0);uint8_t out[64];size_t length;
 for(unsigned i=0;i<92;i++){uint8_t k[4];size_t n=key(i,k);memset(out,0xa5,sizeof out);length=777;CHECK(store_get(&s,k,n,out,sizeof out,&length)==STORE_OK&&length==1&&out[0]==i);memset(out,0xa5,sizeof out);length=777;CHECK(store_get(&s,k,n,out,0,&length)==STORE_FULL&&length==777&&out[0]==0xa5);}
 uint8_t missing=2;length=777;CHECK(store_get(&s,&missing,1,out,sizeof out,&length)==STORE_NOT_FOUND&&length==777&&out[0]==0xa5);CHECK(store_get(&s,&missing,0,out,sizeof out,&length)==STORE_INVALID&&length==777);
 /* Every internal separator must retrieve its equal right-subtree minimum. */
 for(unsigned id=1;id<32;id++)if((s.used&(UINT32_C(1)<<id))&&!s.scratch[id].leaf)for(unsigned i=0;i<s.scratch[id].n;i++)CHECK(store_get(&s,s.scratch[id].key[i],s.scratch[id].klen[i],out,sizeof out,&length)==STORE_OK);
 struct sl_snapshot snap;CHECK(sl_snapshot_capture(&s,&snap)==STORE_OK&&snap.count==92);puts("SL binary validated");return 0;
}
static int normalize(void){
 uint8_t out[16],before[16];size_t length=777;memset(out,0xa5,sizeof out);memcpy(before,out,16);uint8_t bad[]={'A',128};CHECK(sl_normalize(bad,2,out,&length)==STORE_INVALID&&length==777&&memcmp(out,before,16)==0);
 CHECK(sl_normalize((const uint8_t *)"AZ-[az",6,out,&length)==STORE_OK&&length==6&&memcmp(out,"az-[az",6)==0);CHECK(sl_normalize(out,6,out,&length)==STORE_OK&&memcmp(out,"az-[az",6)==0);
 uint8_t maximum[17];memset(maximum,'Z',sizeof maximum);CHECK(sl_normalize(maximum,16,out,&length)==STORE_OK&&length==16&&out[0]=='z'&&out[15]=='z');memcpy(before,out,16);CHECK(sl_normalize(maximum,17,out,&length)==STORE_INVALID&&length==16&&memcmp(before,out,16)==0);
 store s;CHECK(open_empty(&s)==0);struct sl_text text;CHECK(sl_text_bind(&s,&text)==STORE_OK);CHECK(sl_text_put(&text,"Alpha",5,"1",1)==STORE_OK);CHECK(sl_text_put(&text,"ALPHA",5,"2",1)==STORE_OK);CHECK(sl_text_get(&text,"alPHa",5,out,16,&length)==STORE_OK&&length==1&&out[0]=='2');struct store_collected c={.limit=SL_RECORDS};CHECK(sl_text_scan(&text,"A",1,collect,&c)==STORE_OK&&c.count==1&&c.records[0].klen==5&&memcmp(c.records[0].key,"alpha",5)==0);
 CHECK(open_empty(&s)==0);CHECK(store_put(&s,"RAW",3,"v",1)==STORE_OK);struct sl_text refused={0};CHECK(sl_text_bind(&s,&refused)==STORE_INVALID&&refused.engine==NULL);puts("SL normalize validated");return 0;
}
static int scans(void){
 store s;CHECK(setup(&s)==0);struct store_collected full={.limit=SL_RECORDS};CHECK(store_scan(&s,NULL,0,collect,&full)==STORE_OK&&full.count==92);uint8_t low[5][4]={{0},{0,0},{0,0,10},{0,0,10,0},{255,255}};size_t lens[]={0,2,3,4,2};unsigned limits[]={0,1,3,1000};
 for(unsigned j=0;j<5;j++)for(unsigned l=0;l<4;l++){
  struct store_collected c={.limit=limits[l]};CHECK(store_scan(&s,low[j],lens[j],collect,&c)==STORE_OK);size_t expected=0;for(size_t i=0;i<full.count;i++)if(compare(full.records[i].key,full.records[i].klen,low[j],lens[j])>=0&&expected<limits[l]){CHECK(c.records[expected].klen==full.records[i].klen&&memcmp(c.records[expected].key,full.records[i].key,full.records[i].klen)==0);expected++;}CHECK(c.count==expected);CHECK(c.calls==c.count||(limits[l]==0&&c.calls==1));
 }
 struct sl_snapshot snap;CHECK(sl_snapshot_capture(&s,&snap)==STORE_OK);puts("SL scans validated");return 0;
}
struct store_reentry {store *source;size_t calls;int bad;struct sl_snapshot nested;};
static int reenter(void *p,const struct sl_record *record){struct store_reentry *r=p;struct sl_record saved=*record;uint8_t value[64];size_t n=0;if(store_get(r->source,record->key,record->klen,value,sizeof value,&n)!=STORE_OK||n!=record->vlen||memcmp(value,record->value,n))r->bad=1;if(store_delete(r->source,record->key,record->klen)!=STORE_OK||store_check(r->source))r->bad=1;if(!r->calls&&sl_snapshot_capture(r->source,&r->nested)!=STORE_OK)r->bad=1;if(memcmp(&saved,record,sizeof saved))r->bad=1;r->calls++;return 0;}
static int snapshot(void){
 store s;CHECK(setup(&s)==0);struct sl_snapshot snap;CHECK(sl_snapshot_capture(&s,&snap)==STORE_OK&&snap.from_transaction&&snap.count==92);struct store_reentry context={.source=&s};size_t calls=777;CHECK(sl_snapshot_scan(&snap,NULL,0,0,reenter,&context,&calls)==STORE_OK&&calls==0&&context.calls==0);CHECK(sl_snapshot_scan(&snap,NULL,0,SL_RECORDS,reenter,&context,&calls)==STORE_OK&&calls==92&&context.calls==92&&!context.bad&&context.nested.count==91);CHECK(s.used==3&&snap.count==92&&snap.records[91].value[0]==91);
 puts("SL snapshot validated");return 0;
}
static int sl_collect(void *p,const struct sl_record *r){return collect(p,r->key,r->klen,r->value,r->vlen);}
static int lower(void){store s;CHECK(setup(&s)==0);struct sl_snapshot snap;CHECK(sl_snapshot_capture(&s,&snap)==STORE_OK);uint8_t lo[]={0,0,10};struct store_collected c={.limit=SL_RECORDS};size_t calls=777;CHECK(sl_snapshot_scan(&snap,lo,3,SL_RECORDS,sl_collect,&c,&calls)==STORE_OK&&calls==c.count&&c.count>0&&c.records[0].klen==3&&memcmp(c.records[0].key,lo,3)==0);puts("SL lower validated");return 0;}
static int structural(void){
 store s;CHECK(setup(&s)==0);struct sl_snapshot out,before;memset(&out,0xa5,sizeof out);memcpy(&before,&out,sizeof out);
 for(unsigned mode=0;mode<4;mode++){
  store bad=s;unsigned first=0;for(unsigned i=1;i<32;i++)if((bad.used&(UINT32_C(1)<<i))&&bad.scratch[i].leaf){first=i;break;}CHECK(first!=0&&!bad.scratch[bad.root].leaf);
  if(mode==0)bad.scratch[bad.root].child[0]=32;
  if(mode==1)bad.scratch[first].next=first;
  if(mode==2){CHECK(!(bad.used&(UINT32_C(1)<<31)));bad.used|=UINT32_C(1)<<31;bad.scratch[31].leaf=1;}
  if(mode==3)bad.scratch[bad.root].child[1]=bad.scratch[bad.root].child[0];
  reads=0;CHECK(store_check(&bad)==STORE_CORRUPT);CHECK(sl_snapshot_capture(&bad,&out)==STORE_CORRUPT&&memcmp(&out,&before,sizeof out)==0&&reads==0);
  if(mode<2){struct store_collected c={.limit=SL_RECORDS};CHECK(store_scan(&bad,NULL,0,collect,&c)==STORE_CORRUPT&&c.count<=7&&reads==0);}
 }
 puts("SL structural validated");return 0;
}
static int normalize_oracle(void){unsigned a,b,n;while(scanf("%u %u %u",&a,&b,&n)==3){uint8_t input[2]={(uint8_t)a,(uint8_t)b},out[16];size_t length=777;memset(out,0xa5,16);int r=sl_normalize(input,n,out,&length);printf("%d %zu %u %u %u\n",r,length,out[0],out[1],out[15]);}return 0;}
static int records(void){store s;CHECK(setup(&s)==0);struct sl_snapshot snap;CHECK(sl_snapshot_capture(&s,&snap)==STORE_OK);for(size_t i=0;i<snap.count;i++){struct sl_record *r=&snap.records[i];printf("%u ",r->klen);for(unsigned j=0;j<r->klen;j++)printf("%02x",r->key[j]);printf(" %u\n",r->value[0]);}return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;
 if(!strcmp(argv[1],"binary"))return binary();
 if(!strcmp(argv[1],"normalize"))return normalize();
 if(!strcmp(argv[1],"scans"))return scans();
 if(!strcmp(argv[1],"snapshot"))return snapshot();
 if(!strcmp(argv[1],"lower"))return lower();
 if(!strcmp(argv[1],"structural"))return structural();
 if(!strcmp(argv[1],"oracle"))return normalize_oracle();
 if(!strcmp(argv[1],"records"))return records();
 return 2;
}
