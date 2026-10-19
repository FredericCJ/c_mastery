#include "tree.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TREE_CHECK line%d: %s\n",__LINE__,#x);return 1;}}while(0)
static unsigned events[TREE_EVENTS][2],allocations,fail_at;static store *observed;
static uint8_t disk[67][4096];
int tree_page_admit(uint32_t id,int leaf){(void)id;(void)leaf;allocations++;return allocations!=fail_at;}
void tree_event(unsigned event,uint32_t id){if(event<TREE_EVENTS&&id<32)events[event][observed&&observed->scratch[id].leaf?1:0]++;}
static int rd(void *p,uint32_t id,void *out){(void)p;if(id>=67)return 1;memcpy(out,disk[id],4096);return 0;}
static int wr(void *p,uint32_t id,const void *in){(void)p;if(id>=67)return 1;memcpy(disk[id],in,4096);return 0;}
static int fl(void *p){(void)p;return 0;}
static int empty(store *s){store_io io={NULL,rd,wr,fl,67};fail_at=0;memset(disk,0,sizeof disk);observed=s;return store_format(io)||store_open(s,io)||store_begin(s);}
static int model[128];
typedef struct {unsigned next,count;int bad;} Scan;
static int visit(void *ctx,const uint8_t *key,size_t n,const uint8_t *value,size_t z){Scan *v=ctx;while(v->next<128&&model[v->next]<0)v->next++;if(n!=1||z!=1||v->next>=128||key[0]!=v->next||value[0]!=model[v->next])v->bad=1;v->next++;v->count++;return 0;}
static int verify(store *s){if(store_check(s))return 1;unsigned count=0;for(unsigned i=0;i<128;i++){uint8_t k=(uint8_t)i,v=0xa5;size_t n=777;int r=store_get(s,&k,1,&v,1,&n);if(model[i]<0){if(r!=STORE_NOT_FOUND||v!=0xa5||n!=777)return 1;}else{count++;if(r||n!=1||v!=model[i])return 1;}}Scan q={0};return store_scan(s,NULL,0,visit,&q)||q.bad||q.count!=count;}
static unsigned total(unsigned event){return events[event][0]+events[event][1];}
static int update(store *s,unsigned k,int value){uint8_t key=(uint8_t)k,v=(uint8_t)value;int r=value<0?store_delete(s,&key,1):store_put(s,&key,1,&v,1);if(r==STORE_OK)model[k]=value;return r;}
static int exercise(int metrics){
 unsigned count=TREE_CAPACITY==7?92:32;unsigned state=0x41a7;
 memset(events,0,sizeof events);
 for(unsigned pattern=0;pattern<12;pattern++){
  store s;CHECK(empty(&s)==0);for(unsigned i=0;i<128;i++)model[i]=-1;
  for(unsigned i=0;i<count;i++){unsigned k=pattern%3==0?i:pattern%3==1?count-1-i:(i%2?count-1-i/2:i/2);CHECK(update(&s,k,(int)k)==STORE_OK&&verify(&s)==0);}
  CHECK(total(TREE_INTERNAL_SPLIT)>0&&total(TREE_ROOT_GROW)>1);
  unsigned order[92];for(unsigned i=0;i<count;i++)order[i]=(37*i)%count;
  if(pattern>=3)for(unsigned i=count-1;i>0;i--){state=state*1664525u+1013904223u;unsigned j=state%(i+1);unsigned temp=order[i];order[i]=order[j];order[j]=temp;}
  for(unsigned i=0;i<count;i++){CHECK(update(&s,order[i],-1)==STORE_OK&&verify(&s)==0);}
  CHECK(s.used==3&&s.root==1);
  for(unsigned i=0;i<count;i++)CHECK(update(&s,i,(int)(127-i))==STORE_OK&&verify(&s)==0);
 }
 CHECK(total(TREE_BORROW_LEFT)>0&&total(TREE_BORROW_RIGHT)>0&&total(TREE_MERGE_LEFT)>0&&total(TREE_MERGE_RIGHT)>0&&total(TREE_ROOT_SHRINK)>0);
 for(unsigned kind=0;kind<2;kind++)CHECK(events[TREE_BORROW_LEFT][kind]&&events[TREE_BORROW_RIGHT][kind]&&events[TREE_MERGE_LEFT][kind]&&events[TREE_MERGE_RIGHT][kind]);
 if(metrics)printf("{\"capacity\":%u,\"leaf_splits\":%u,\"internal_splits\":%u,\"left\":%u,\"right\":%u,\"merge_left\":%u,\"merge_right\":%u,\"shrink\":%u,\"minimum_visits\":%u,\"full_visits\":%u,\"repair_nodes\":%u,\"store_bytes\":%zu}\n",TREE_CAPACITY,total(TREE_LEAF_SPLIT),total(TREE_INTERNAL_SPLIT),total(TREE_BORROW_LEFT),total(TREE_BORROW_RIGHT),total(TREE_MERGE_LEFT),total(TREE_MERGE_RIGHT),total(TREE_ROOT_SHRINK),total(TREE_MIN_VISIT),total(TREE_FULL_VISIT),total(TREE_REPAIR_NODE),sizeof(store));
 else {printf("TREE exercise validated capacity=%u all-leaf-and-internal-events\n",TREE_CAPACITY);}return 0;
}
static int minimum_change(void){store s;CHECK(empty(&s)==0);for(unsigned i=0;i<128;i++)model[i]=-1;unsigned count=TREE_CAPACITY==7?27:11;for(unsigned i=0;i<count;i++)CHECK(update(&s,i,(int)i)==STORE_OK);CHECK(verify(&s)==0);unsigned target=TREE_CAPACITY==7?20:8;CHECK(update(&s,target,-1)==STORE_OK&&verify(&s)==0);puts("TREE minimum validated");return 0;}
static int binary_separator(void){store s;CHECK(empty(&s)==0);for(unsigned i=0;i<128;i++)model[i]=-1;unsigned count=TREE_CAPACITY==7?40:20;for(unsigned i=0;i<count;i++)CHECK(update(&s,i,(int)i)==STORE_OK);for(unsigned id=1;id<32;id++)if((s.used&(UINT32_C(1)<<id))&&!s.scratch[id].leaf)for(unsigned i=0;i<s.scratch[id].n;i++){uint8_t out;size_t n;CHECK(store_get(&s,s.scratch[id].key[i],s.scratch[id].klen[i],&out,1,&n)==STORE_OK&&n==1&&out==s.scratch[id].key[i][0]);}puts("TREE separator validated");return 0;}
static int oracle(void){store seed,s;CHECK(empty(&seed)==0);unsigned reset,op,k,value;while(scanf("%u %u %u %u",&reset,&op,&k,&value)==4){if(reset)s=seed;observed=&s;uint8_t key=(uint8_t)k,v=(uint8_t)value;int r=op?store_put(&s,&key,1,&v,1):store_delete(&s,&key,1);CHECK(store_check(&s)==STORE_OK);printf("%d",r);for(unsigned i=0;i<5;i++){uint8_t q=(uint8_t)i,out=0;size_t n=0;int got=store_get(&s,&q,1,&out,1,&n);CHECK(got==STORE_OK||got==STORE_NOT_FOUND);printf(" %d",got==STORE_OK?(int)out:-1);}putchar('\n');}return 0;}
static int shape(void){store s;CHECK(empty(&s)==0);for(unsigned i=0;i<128;i++)model[i]=-1;unsigned half=(TREE_CAPACITY+1)/2;for(unsigned i=0;i<=TREE_CAPACITY;i++)CHECK(update(&s,i,(int)i)==STORE_OK);store_node *root=&s.scratch[s.root],*left=&s.scratch[root->child[0]],*right=&s.scratch[root->child[1]];CHECK(!root->leaf&&root->n==1&&root->key[0][0]==half&&left->n==half&&right->n==half&&left->next==root->child[1]&&right->next==0);CHECK(left->key[half-1][0]==half-1&&right->key[0][0]==half&&verify(&s)==0);puts("TREE shape validated copied-leaf-separator");return 0;}
static int mixed(void){store s;CHECK(empty(&s)==0);for(unsigned i=0;i<128;i++)model[i]=-1;unsigned rng=0x713a,put=0,replace=0,missing=0,removed=0;for(unsigned i=0;i<4096;i++){rng=rng*1664525u+1013904223u;unsigned key=(rng>>8)%20;rng=rng*1664525u+1013904223u;int value=(rng&3)?(int)((rng>>16)&127):-1;int existed=model[key]>=0;int r=update(&s,key,value);CHECK(r==(value<0&&!existed?STORE_NOT_FOUND:STORE_OK));if(value<0){if(existed)removed++;else missing++;}else if(existed)replace++;else put++;CHECK(verify(&s)==0);}CHECK(put&&replace&&missing&&removed);puts("TREE mixed validated operations=4096");return 0;}
static int persistence(void){store s,r;CHECK(empty(&s)==0);for(unsigned i=0;i<128;i++)model[i]=-1;unsigned count=TREE_CAPACITY==7?92:32;for(unsigned i=0;i<count;i++)CHECK(update(&s,i,(int)i)==STORE_OK);CHECK(store_commit(&s)==STORE_OK&&store_open(&r,s.io)==STORE_OK);observed=&r;CHECK(verify(&r)==0&&store_begin(&r)==STORE_OK);for(unsigned i=0;i<count;i++)CHECK(update(&r,(37*i)%count,-1)==STORE_OK);CHECK(store_commit(&r)==STORE_OK&&store_open(&s,r.io)==STORE_OK);observed=&s;CHECK(verify(&s)==0&&store_begin(&s)==STORE_OK);for(unsigned i=0;i<count;i++)CHECK(update(&s,i,127-(int)i)==STORE_OK);CHECK(store_commit(&s)==STORE_OK&&store_open(&r,s.io)==STORE_OK);observed=&r;CHECK(verify(&r)==0);puts("TREE persistence validated commit-open-delete-reuse");return 0;}
static int page(void){store_node n={0};n.leaf=1;n.n=1;n.klen[0]=1;n.vlen[0]=1;n.key[0][0]=0x80;n.value[0][0]=0xa5;uint8_t bytes[4096];CHECK(store_page_encode(1,7,&n,bytes)==STORE_OK);for(unsigned i=0;i<4096;i++)printf("%02x",bytes[i]);putchar('\n');return 0;}
static int foreign(void){uint8_t bytes[4096];for(unsigned i=0;i<4096;i++){unsigned v;CHECK(scanf("%2x",&v)==1);bytes[i]=(uint8_t)v;}store_node out,before;memset(&out,0xa5,sizeof out);before=out;CHECK(store_page_decode(1,7,bytes,&out)==STORE_CORRUPT&&memcmp(&out,&before,sizeof out)==0);puts("TREE foreign validated format-rejection");return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"exercise"))return exercise(0);if(!strcmp(argv[1],"metrics"))return exercise(1);if(!strcmp(argv[1],"minimum"))return minimum_change();if(!strcmp(argv[1],"separator"))return binary_separator();if(!strcmp(argv[1],"oracle"))return oracle();if(!strcmp(argv[1],"shape"))return shape();if(!strcmp(argv[1],"mixed"))return mixed();if(!strcmp(argv[1],"persistence"))return persistence();if(!strcmp(argv[1],"page"))return page();if(!strcmp(argv[1],"foreign"))return foreign();return 2;}
