#include "core.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"B06_CHECK %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;} } while(0)
static int test_program(void) {
unsigned char d[2]={255,240},s[2]={15,224};CHECK(lc_program(d,s,2)&&d[0]==15&&d[1]==224);s[0]=7;s[1]=255;CHECK(!lc_program(d,s,2)&&d[0]==15);CHECK(lc_program(d,s,0));
return 0;
}
static int test_crc(void) {
CHECK(lc_crc((const unsigned char*)"",0)==0);CHECK(lc_crc((const unsigned char*)"123456789",9)==UINT32_C(0xcbf43926));
return 0;
}
static int test_header(void) {
CHECK(lc_admit(32,64,4,3,1));CHECK(!lc_admit(0,64,4,3,1));CHECK(!lc_admit(65,64,4,3,1));CHECK(!lc_admit(32,64,2,3,1));CHECK(!lc_admit(32,64,4,3,0));CHECK(!lc_admit(32,64,4,3,2));
return 0;
}
static int test_boot(void) {
struct lc_boot s={0,1,0,0};unsigned slot=9;CHECK(lc_select(&s,&slot)&&slot==1&&s.attempted);CHECK(lc_select(&s,&slot)&&slot==0);s.confirmed=1;CHECK(lc_select(&s,&slot)&&slot==0);s.active=2;CHECK(!lc_select(&s,&slot));
return 0;
}
static int test_journal(void) {
struct lc_journal a={1,7,0,0xa5},b={2,9,0,0};unsigned i=9;a.check=a.seq^a.value^UINT32_C(0x5a5a5a5a);b.check=b.seq^b.value^UINT32_C(0x5a5a5a5a);CHECK(lc_journal_select(&a,&b,&i)&&i==0);b.commit=0xa5;CHECK(lc_journal_select(&a,&b,&i)&&i==1);b.seq=1;b.check=b.seq^b.value^UINT32_C(0x5a5a5a5a);CHECK(!lc_journal_select(&a,&b,&i));
return 0;
}
static int test_floor(void) {
uint32_t o=9;CHECK(lc_raise_floor(4,5,1,&o)&&o==5);CHECK(!lc_raise_floor(4,3,1,&o)&&o==5);CHECK(!lc_raise_floor(4,5,0,&o));CHECK(lc_raise_floor(4,4,1,&o));
return 0;
}
static int test_peer(void) {
CHECK(lc_peer_ready(4,4,0));CHECK(!lc_peer_ready(4,3,0));CHECK(!lc_peer_ready(4,4,1));
return 0;
}
static int test_capture(void) {
uint32_t p[4]={3,4096,0,0xc0de};p[2]=p[0]^p[1]^UINT32_C(0x9e3779b9);CHECK(lc_capture_valid(p,4));p[2]^=1;CHECK(!lc_capture_valid(p,4));CHECK(!lc_capture_valid(p,3));
return 0;
}
int main(int argc,char **argv) { if(argc!=2) return 2;
if(strcmp(argv[1],"program")==0) return test_program();
if(strcmp(argv[1],"crc")==0) return test_crc();
if(strcmp(argv[1],"header")==0) return test_header();
if(strcmp(argv[1],"boot")==0) return test_boot();
if(strcmp(argv[1],"journal")==0) return test_journal();
if(strcmp(argv[1],"floor")==0) return test_floor();
if(strcmp(argv[1],"peer")==0) return test_peer();
if(strcmp(argv[1],"capture")==0) return test_capture();
return 2; }
