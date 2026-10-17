#include "core.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"B06_CHECK %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;} } while(0)
static int test_span(void) {
unsigned char a[32]={0},p[2]={65,66};a[31]=90;CHECK(md_write(a,31,29,p,2));CHECK(!md_write(a,31,30,p,2)&&a[31]==90);CHECK(!md_write(a,31,SIZE_MAX,p,1));CHECK(md_write(a,0,0,p,0));
return 0;
}
static int test_extent(void) {
uint32_t o=99;CHECK(md_extent(3,8,24,&o)&&o==24);CHECK(!md_extent(8193,8,64,&o)&&o==24);CHECK(!md_extent(UINT32_MAX,2,UINT32_MAX,&o));CHECK(md_extent(0,8,0,&o)&&o==0);
return 0;
}
static int test_generation(void) {
MDArena a={2,1,7,{77}};unsigned char o=99;CHECK(md_read(&a,2,0,&o)&&o==77);o=99;CHECK(!md_read(&a,1,0,&o)&&o==99);a.live=0;CHECK(!md_read(&a,2,0,&o));
return 0;
}
static int test_arena(void) {
MDArena a={2,1,7,{0}};CHECK(md_authorize(&a,7,2,15,1));CHECK(!md_authorize(&a,8,2,0,1));CHECK(!md_authorize(&a,7,1,0,1));CHECK(!md_authorize(&a,7,2,16,1));CHECK(md_authorize(&a,7,2,16,0));
return 0;
}
static int test_dispatch(void) {
unsigned s=9;CHECK(md_dispatch(0,0,&s)&&s==9);CHECK(!md_dispatch(1,0,&s)&&s==9);CHECK(md_dispatch(1,1,&s)&&s==77);CHECK(!md_dispatch(2,1,&s));
return 0;
}
static int test_format(void) {
char out[8]={'Q'};CHECK(md_log_copy("%n",2,out,8)&&out[0]=='%'&&out[1]=='n'&&out[2]==0);out[0]='Q';CHECK(!md_log_copy("abcdefghi",9,out,8)&&out[0]=='Q');
return 0;
}
static int test_release(void) {
MDOwner o={1,0};CHECK(md_release(&o)&&o.owned==0&&o.releases==1);CHECK(!md_release(&o)&&o.releases==1);o.owned=1;o.releases=UINT_MAX;CHECK(!md_release(&o)&&o.owned==1);
return 0;
}
static int test_compare(void) {
unsigned char a[]={1,2,3},b[]={9,2,3};size_t steps;CHECK(!md_equal(a,b,3,&steps)&&steps==3);CHECK(md_equal(a,a,3,&steps)&&steps==3);CHECK(md_equal(0,0,0,&steps)&&steps==0);
return 0;
}
int main(int argc,char **argv) { if(argc!=2) return 2;
if(strcmp(argv[1],"span")==0) return test_span();
if(strcmp(argv[1],"extent")==0) return test_extent();
if(strcmp(argv[1],"generation")==0) return test_generation();
if(strcmp(argv[1],"arena")==0) return test_arena();
if(strcmp(argv[1],"dispatch")==0) return test_dispatch();
if(strcmp(argv[1],"format")==0) return test_format();
if(strcmp(argv[1],"release")==0) return test_release();
if(strcmp(argv[1],"compare")==0) return test_compare();
return 2; }
