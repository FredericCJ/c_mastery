#include "b04.h"
#include <limits.h>
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"DEPTH_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static int serial(void){
 for(unsigned a=0;a<16;a++)for(unsigned b=0;b<16;b++){
  int out=77;unsigned forward=(b+16-a)%16;
  CHECK(b4_serial(a,b,4,&out)==(forward==8?B4_CONFLICT:B4_OK));
  CHECK(out==(forward==8?77:forward==0?0:forward<8?-1:1));
  for(unsigned n=0;n<8;n++){int hit=77;CHECK(!b4_window(a,b,n,4,&hit));CHECK(hit==(forward<n));}
 }
 int out=77;CHECK(!b4_serial(UINT32_MAX,0,32,&out)&&out==-1);
 CHECK(b4_serial(0,UINT32_C(0x80000000),32,&out)==B4_CONFLICT&&out==-1);
 CHECK(b4_window(0,1,8,4,&out)==B4_RANGE);return 0;
}
static int certificate(void){
 B4Graph g;g.vertices=3;for(unsigned i=0;i<6;i++)for(unsigned j=0;j<6;j++)g.cost[i][j]=UINT32_MAX;
 g.cost[0][1]=8;g.cost[0][2]=1;g.cost[2][1]=1;
 uint32_t d[]={0,2,1};unsigned p[]={UINT_MAX,2,0};CHECK(!b4_certificate(&g,0,d,p));
 d[1]=8;p[1]=0;CHECK(b4_certificate(&g,0,d,p)==B4_CONFLICT);
 d[1]=2;p[1]=2;g.cost[2][1]=9;CHECK(b4_certificate(&g,0,d,p)==B4_CONFLICT);
 g.cost[0][1]=g.cost[0][2]=UINT32_MAX;g.cost[1][2]=g.cost[2][1]=0;
 d[1]=d[2]=0;p[1]=2;p[2]=1;CHECK(b4_certificate(&g,0,d,p)==B4_CONFLICT);
 d[1]=d[2]=UINT32_MAX;p[1]=p[2]=UINT_MAX;CHECK(!b4_certificate(&g,0,d,p));
 g.cost[0][1]=1000001;CHECK(b4_certificate(&g,0,d,p)==B4_RANGE);return 0;
}
static int framing(void){
 /* Original local differential: each toy recipient trusts a different field.
  * The body is synthetic; no network request or third-party parser is used. */
 const unsigned char body[]="0\r\n\r\nSYNTHETIC";
 size_t cl_first_boundary=6,te_first_boundary=0;
 if(body[0]=='0'&&body[1]=='\r'&&body[2]=='\n'&&body[3]=='\r'&&body[4]=='\n') te_first_boundary=5;
 CHECK(cl_first_boundary!=te_first_boundary&&body[cl_first_boundary-1]=='S');
 const unsigned char conflict[]="Content-Length: 6\r\nTransfer-Encoding: chunked\r\n\r\n";
 B4Framing unchanged={9,99};
 CHECK(b4_framing(conflict,sizeof conflict-1,16,&unchanged)==B4_CONFLICT&&unchanged.chunked==9&&unchanged.body==99);
 struct{const char*s;int status;unsigned chunked;size_t body;}v[]={
 {"\r\n",B4_OK,0,0},{"Content-Length: 3\r\n\r\n",B4_OK,0,3},
 {"tRaNsFeR-EnCoDiNg: chunked\r\n\r\n",B4_OK,1,0},
 {"Content-Length: 3\r\nTransfer-Encoding: chunked\r\n\r\n",B4_CONFLICT,0,0},
 {"Content-Length: 0\r\nContent-Length: 0\r\n\r\n",B4_CONFLICT,0,0},
 {"Content-Length: 17\r\n\r\n",B4_RANGE,0,0},
 {"Content-Length: 184467440737095516160\r\n\r\n",B4_RANGE,0,0},
 {"Content-Length : 1\r\n\r\n",B4_INVALID,0,0},
 {"Content-Length: +1\r\n\r\n",B4_INVALID,0,0},
 {"Transfer-Encoding: chunked, gzip\r\n\r\n",B4_CONFLICT,0,0},
 {"Content-Length: 1\n\n",B4_INVALID,0,0},{"\r\nX",B4_INVALID,0,0}};
 for(size_t i=0;i<sizeof v/sizeof*v;i++){B4Framing r={9,99};CHECK(b4_framing((const unsigned char*)v[i].s,strlen(v[i].s),16,&r)==v[i].status);if(v[i].status)CHECK(r.chunked==9&&r.body==99);else CHECK(r.chunked==v[i].chunked&&r.body==v[i].body);}
 const unsigned char s[]="Content-Length: 3\r\n\r\n";
 for(size_t n=0;n<sizeof s-1;n++){B4Framing r={9,99};CHECK(b4_framing(s,n,16,&r)!=B4_OK&&r.body==99);}
 return 0;
}
static int dma(void){
 B4Dma d={B4_CPU,0,1,1};CHECK(!b4_dma(&d,B4_WRITE,1));
 CHECK(b4_dma(&d,B4_SUBMIT,1)==B4_STATE&&d.owner==B4_CPU&&d.dirty);
 CHECK(!b4_dma(&d,B4_CLEAN,1));CHECK(!b4_dma(&d,B4_SUBMIT,1));
 CHECK(b4_dma(&d,B4_WRITE,1)==B4_STATE);CHECK(b4_dma(&d,B4_COMPLETE,0)==B4_STATE);
 CHECK(!b4_dma(&d,B4_COMPLETE,1));CHECK(b4_dma(&d,B4_RELEASE,1)==B4_STATE);
 CHECK(!b4_dma(&d,B4_INVALIDATE,1));CHECK(!b4_dma(&d,B4_RELEASE,1));CHECK(d.generation==2);
 for(size_t off=0;off<=64;off++)for(size_t n=0;n<=64-off;n++){
  size_t begin=99,end=99;CHECK(!b4_cache_extent(off,n,16,64,&begin,&end));
  CHECK(begin==(n?off/16*16:off));CHECK(end==(n?(off+n+15)/16*16:off));
 }
 size_t b=99,e=99;CHECK(b4_cache_extent(SIZE_MAX,1,16,SIZE_MAX,&b,&e)==B4_RANGE&&b==99&&e==99);
 CHECK(b4_cache_extent(0,1,3,64,&b,&e)==B4_INVALID);return 0;
}
static int memory(void){
 B4Ledger l={0};CHECK(!b4_memory(&l,B4_ALLOC,1,10,1));CHECK(l.live==10&&l.peak==10);
 CHECK(!b4_memory(&l,B4_RESIZE,1,20,0));CHECK(l.live==10&&l.slots[0].live&&l.peak==10);
 CHECK(!b4_memory(&l,B4_ALLOC,2,5,1));CHECK(!b4_memory(&l,B4_RESIZE,1,2,1));CHECK(l.live==7&&l.peak==15);
 CHECK(!b4_memory(&l,B4_FREE,2,0,1));CHECK(l.live==2&&l.peak==15);
 CHECK(b4_memory(&l,B4_FREE,2,0,1)==B4_STATE&&l.live==2);
 CHECK(b4_memory(&l,B4_ALLOC,3,SIZE_MAX,1)==B4_RANGE&&l.live==2);
 CHECK(!b4_memory(&l,B4_FREE,1,0,1)&&l.live==0&&l.peak==15);
 CHECK(!b4_memory(&l,B4_ALLOC,1,1,0)&&l.live==0);
 for(unsigned i=1;i<=4;i++) CHECK(!b4_memory(&l,B4_ALLOC,i,0,1));
 CHECK(b4_memory(&l,B4_ALLOC,5,0,1)==B4_RANGE&&l.live==0);
 CHECK(b4_memory(&l,B4_ALLOC,1,1,1)==B4_STATE&&l.live==0);
 CHECK(b4_memory(&l,B4_RESIZE,9,2,1)==B4_STATE&&l.live==0);return 0;
}
int main(int n,char**a){if(n!=2)return 2;
 const struct{const char*n;int(*f)(void);}c[]={{"serial",serial},{"certificate",certificate},{"framing",framing},{"dma",dma},{"memory",memory}};
 for(size_t i=0;i<sizeof c/sizeof*c;i++) { if(!strcmp(a[1],c[i].n)) return c[i].f(); }
 return 2;}
