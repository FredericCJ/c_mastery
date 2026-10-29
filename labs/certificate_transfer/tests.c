#include "transfer.h"
#include <limits.h>
void *memcpy(void*,const void*,size_t);
void *memset(void*,int,size_t);
static int ct_compare(const void*x,const void*y,size_t n){const unsigned char*a=x,*b=y;for(size_t i=0;i<n;++i)if(a[i]!=b[i])return 1;return 0;}
#define memcmp ct_compare
#define CHECK(x) do{if(!(x))return ct_failure(#x,__LINE__);}while(0)
static const unsigned char wire[]={0x47,0x43,1,3,3,0,0,1,0,1,0,2,0,2,2,1,0xff,0xfd};
static struct ag_certificate certificate(void){struct ag_certificate c={.reachable=7};c.distance[1]=-1;c.distance[2]=2;c.previous[0]=0;c.previous[1]=2;c.previous[2]=0;return c;}
static int stream(void){
 struct ag_certificate c=certificate();struct ct_stream s;struct cd_csr out,before;memset(&before,0x5a,sizeof before);unsigned cases=0;
 for(size_t a=0;a<=sizeof wire;++a)for(size_t b=a;b<=sizeof wire;++b){ct_init(&s);memcpy(&out,&before,sizeof out);
  CHECK(ct_feed(&s,wire,a,0,&c,&out)==CT_MORE);CHECK(!memcmp(&out,&before,sizeof out));
  CHECK(ct_feed(&s,wire+a,b-a,0,&c,&out)==CT_MORE);CHECK(!memcmp(&out,&before,sizeof out));
  CHECK(ct_feed(&s,wire+b,sizeof wire-b,1,&c,&out)==CT_OK);CHECK(out.vertices==3&&out.count==3&&out.edges[2].weight==-3);
  CHECK(ct_feed(&s,NULL,0,1,&c,&out)==CT_STATE);++cases;
 }
 for(size_t n=0;n<sizeof wire;++n){ct_init(&s);memcpy(&out,&before,sizeof out);CHECK(ct_feed(&s,wire,n,1,&c,&out)==CT_TRUNCATED);CHECK(!memcmp(&out,&before,sizeof out));CHECK(ct_feed(&s,wire,sizeof wire,1,&c,&out)==CT_STATE);++cases;}
 for(size_t n=0;n<=sizeof wire;++n){ct_init(&s);CHECK(ct_feed(&s,wire,n,0,&c,&out)==CT_MORE);ct_init(&s);CHECK(ct_feed(&s,wire,sizeof wire,1,&c,&out)==CT_OK);++cases;}
 unsigned char bad[103];memcpy(bad,wire,sizeof wire);bad[sizeof wire]=0;
 ct_init(&s);memcpy(&out,&before,sizeof out);CHECK(ct_feed(&s,bad,sizeof wire+1,1,&c,&out)==CT_INVALID);CHECK(!memcmp(&out,&before,sizeof out));++cases;
 for(unsigned i=0;i<8;++i){memcpy(bad,wire,sizeof wire);const unsigned at[]={0,1,2,3,4,5,6,7};bad[at[i]]=255;ct_init(&s);memcpy(&out,&before,sizeof out);CHECK(ct_feed(&s,bad,sizeof wire,1,&c,&out)==CT_INVALID);CHECK(!memcmp(&out,&before,sizeof out));++cases;}
 c.distance[1]=0;ct_init(&s);memcpy(&out,&before,sizeof out);CHECK(ct_feed(&s,wire,sizeof wire,1,&c,&out)==CT_CERTIFICATE);CHECK(!memcmp(&out,&before,sizeof out));++cases;
 memset(bad,0,sizeof bad);bad[0]=0x47;bad[1]=0x43;bad[2]=1;bad[3]=1;bad[4]=24;c=(struct ag_certificate){.reachable=1};
 ct_init(&s);CHECK(ct_feed(&s,bad,102,1,&c,&out)==CT_OK);CHECK(out.count==24);++cases;
 ct_init(&s);memcpy(&out,&before,sizeof out);CHECK(ct_feed(&s,bad,103,1,&c,&out)==CT_INVALID);CHECK(!memcmp(&out,&before,sizeof out));++cases;
 bad[4]=0;ct_init(&s);CHECK(ct_feed(&s,bad,6,1,&c,&out)==CT_OK);CHECK(out.count==0);++cases;
 ct_observation("stream",cases,(unsigned)sizeof s);return 0;
}
static struct at_certificate balanced(void){
 struct at_certificate c={.root=0,.capacity=64,.count=7};const int keys[]={40,20,60,10,30,50,70};
 for(unsigned i=0;i<7;++i)c.nodes[i]=(struct at_node){.key=keys[i],.left=i<3?(int)(2*i+1):-1,.right=i<3?(int)(2*i+2):-1,.parent=i?(int)((i-1)/2):-1,.height=i==0?3:i<3?2:1,.live=1};
 return c;
}
static int pair(const struct at_certificate*c,unsigned policy,unsigned*cases){struct ct_workspace w;int a=1234,b=1234;int x=at_check(c,policy,&a),y=ct_tree(c,policy,&w,64,&b);CHECK(x==y);CHECK(a==b);CHECK(w.high_water<=64);++*cases;return 0;}
static int tree(void){
 struct at_certificate c=balanced();struct ct_workspace w;int h=1234;unsigned cases=0;
 CHECK(ct_tree(&c,1,&w,64,&h)==ALGO_OK&&h==3&&w.high_water==3);++cases;
 c.nodes[0].height=2;h=1234;CHECK(ct_tree(&c,1,&w,64,&h)==ALGO_BAD&&h==1234);++cases;
 c=balanced();c.nodes[4].key=45;CHECK(ct_tree(&c,0,&w,64,&h)==ALGO_BAD);++cases;
 c=balanced();h=1234;CHECK(ct_tree(&c,1,&w,2,&h)==CT_SPACE&&h==1234);++cases;
 for(unsigned policy=0;policy<3;++policy){c=balanced();CHECK(!pair(&c,policy,&cases));
  for(unsigned node=0;node<7;++node)for(unsigned field=0;field<7;++field)for(int value=-2;value<9;++value){c=balanced();struct at_node*n=&c.nodes[node];switch(field){case 0:n->left=value;break;case 1:n->right=value;break;case 2:n->parent=value;break;case 3:n->height=value;break;case 4:n->key=value*10;break;case 5:n->red=(unsigned)value;break;default:n->live=(unsigned)value;break;}CHECK(!pair(&c,policy,&cases));}
 }
 c=(struct at_certificate){.root=0,.capacity=64,.count=64};for(int i=0;i<64;++i)c.nodes[i]=(struct at_node){.key=i,.left=-1,.right=i==63?-1:i+1,.parent=i-1,.height=64-i,.live=1};
 CHECK(!pair(&c,0,&cases));h=1234;CHECK(ct_tree(&c,0,&w,64,&h)==ALGO_OK&&h==64&&w.high_water==64);CHECK(ct_tree(&c,0,&w,63,&h)==CT_SPACE);++cases;
 c=(struct at_certificate){.root=-1,.capacity=64};CHECK(ct_tree(&c,0,&w,0,&h)==ALGO_OK&&h==0&&w.high_water==0);++cases;
 c.count=1;h=1234;CHECK(ct_tree(&c,0,&w,0,&h)==ALGO_BAD&&h==1234);++cases;
 c=balanced();c.nodes[3].key=INT32_MIN;c.nodes[6].key=INT32_MAX;CHECK(!pair(&c,1,&cases));
 c.nodes[0].left=0;CHECK(!pair(&c,1,&cases));c=balanced();c.nodes[2].left=3;CHECK(!pair(&c,1,&cases));
 ct_observation("tree",cases,(unsigned)sizeof w);return 0;
}
int ct_tests(unsigned which){return which?tree():stream();}
