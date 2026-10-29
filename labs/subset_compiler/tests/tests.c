#include "core.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"B06_CHECK %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;} } while(0)
static int test_token(void) {
size_t i=0;unsigned k=9;uint32_t v=9;CHECK(cc_token(" 65535",6,&i,&k,&v)&&i==6&&k==1&&v==65535);i=0;CHECK(!cc_token("65536",5,&i,&k,&v)&&i==0);CHECK(!cc_token("?",1,&i,&k,&v));
return 0;
}
static int test_parse(void) {
struct cc_ins ir[32];size_t n=99;CHECK(cc_parse("2+3*x",5,ir,32,&n)&&n==6&&ir[3].op==4&&ir[4].op==3);n=99;CHECK(!cc_parse("2?",2,ir,32,&n)&&n==99);CHECK(!cc_parse("(2",2,ir,32,&n));CHECK(!cc_parse("2+3",3,ir,2,&n));
return 0;
}
static int test_types(void) {
uint32_t o=9;CHECK(cc_binary(3,UINT32_MAX,1,&o)&&o==0);CHECK(cc_binary(4,65535,65535,&o)&&o==UINT32_C(4294836225));CHECK(!cc_binary(7,1,2,&o)&&o==UINT32_C(4294836225));
return 0;
}
static int test_ir(void) {
struct cc_ins ir[]={{2,0},{1,7},{3,0},{5,0}};uint32_t out=99;CHECK(cc_run(ir,4,4,&out)&&out==11);ir[0].op=3;CHECK(!cc_run(ir,4,4,&out)&&out==11);
return 0;
}
static int test_verify(void) {
struct cc_ins a[]={{1,2},{1,3},{3,0},{5,0},{1,9}};unsigned peak=99;CHECK(cc_validate(a,4,&peak)&&peak==2);CHECK(!cc_validate(a,5,&peak));CHECK(!cc_validate(a,3,&peak));a[0].op=3;CHECK(!cc_validate(a,4,&peak));
return 0;
}
static int test_x86(void) {
struct cc_ins a[]={{2,0},{1,3},{4,0},{5,0}};char out[4096];CHECK(cc_x86(a,4,out,sizeof out));CHECK(strstr(out,"imull %ecx,%eax")!=0);out[0]='Q';CHECK(!cc_x86(a,4,out,2)&&out[0]=='Q');
return 0;
}
static int test_thumb(void) {
struct cc_ins a[]={{2,0},{1,3},{4,0},{5,0}};char out[4096];CHECK(cc_thumb(a,4,out,sizeof out));CHECK(strstr(out,"mov r3,r0")&&strstr(out,"mul r0,r1,r0")&&strstr(out,"bx lr"));out[0]='Q';CHECK(!cc_thumb(a,4,out,2)&&out[0]=='Q');
return 0;
}
static int test_differential(void) {
uint32_t out=99;CHECK(cc_compile_eval("(x+2)*3",7,4,&out)&&out==18);CHECK(cc_compile_eval("x*x",3,UINT32_MAX,&out)&&out==1);CHECK(!cc_compile_eval("x/0",3,4,&out)&&out==1);
return 0;
}
int main(int argc,char **argv) { if(argc!=2) return 2;
if(strcmp(argv[1],"token")==0) return test_token();
if(strcmp(argv[1],"parse")==0) return test_parse();
if(strcmp(argv[1],"types")==0) return test_types();
if(strcmp(argv[1],"ir")==0) return test_ir();
if(strcmp(argv[1],"verify")==0) return test_verify();
if(strcmp(argv[1],"x86")==0) return test_x86();
if(strcmp(argv[1],"thumb")==0) return test_thumb();
if(strcmp(argv[1],"differential")==0) return test_differential();
return 2; }
