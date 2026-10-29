#include "capability.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#define CHECK(x) do {if (!(x)) {fprintf(stderr,"LP_CHECK %s:%d %s\n",__FILE__,__LINE__,#x);return 1;}} while(0)
static int record(void) {
    struct lp_value v={.value=7,.present=1}; char b[128]="unchanged";
    CHECK(lp_record(0,INTMAX_MIN,&v) && v.present==0 && v.value==0);
    CHECK(lp_format(v,b,sizeof b) && !strcmp(b,"empty"));
    CHECK(lp_record(1,INTMAX_MIN,&v) && v.value==INTMAX_MIN);
    CHECK(lp_format(v,b,sizeof b));
    CHECK(strtoimax(b,NULL,10)==INTMAX_MIN);
    CHECK(lp_record(1,INTMAX_MAX,&v) && lp_format(v,b,sizeof b));
    CHECK(strtoimax(b,NULL,10)==INTMAX_MAX);
    strcpy(b,"unchanged");CHECK(!lp_format(v,b,2) && !strcmp(b,"unchanged"));
    CHECK(!lp_record(2,0,&v) && v.value==INTMAX_MAX && v.present==1);
    CHECK(!lp_record(1,0,NULL));
    puts("LP record validated");return 0;
}
static int transform(void) {
    int a[3]={1,INT_MAX/2,INT_MAX},b[3]={91,92,93};
    CHECK(!lp_transform(a,b,3) && b[0]==91 && b[1]==92 && b[2]==93);
    a[0]=INT_MIN/2;a[2]=0;
    CHECK(lp_transform(a,b,3) && b[0]==a[0]*2 && b[1]==a[1]*2 && b[2]==0);
    CHECK(lp_transform(NULL,NULL,0) && !lp_transform(NULL,b,1) && !lp_transform(a,b,17));
    puts("LP transform validated");return 0;
}
static int matrix(void) {
    unsigned u=9;
    CHECK(lp_capability(15,3,&u) && u==0);
    CHECK(lp_capability(1,1,&u) && u==1);
    CHECK(lp_capability(0,0,&u) && u==1);
    u=9;CHECK(!lp_capability(16,15,&u) && u==9);
    CHECK(!lp_capability(15,16,&u) && u==9 && !lp_capability(1,1,NULL));
    puts("LP matrix validated");return 0;
}
static unsigned calls;static int first,second;
static int trace_sink(int a,int b) {calls++;first=a;second=b;return a+b;}
static int trace(void) {
    int a=2,b=7;calls=0;
    CHECK(LP_TRACE(trace_sink,a++,b++)==9);
    CHECK(a==3 && b==8 && calls==1 && first==2 && second==7);
    /* _Generic's controlling expression is not evaluated. */
    CHECK(_Generic(a++,int:1,default:0)==1 && a==3);
    puts("LP trace validated");return 0;
}
static int oracle(void) {
    char kind;
    while(scanf(" %c",&kind)==1) {
        if(kind=='m') {unsigned r,o,u=9;CHECK(scanf("%u%u",&r,&o)==2);int ok=lp_capability(r,o,&u);printf("%d %u\n",ok,u);}
        else if(kind=='t') {int a[3],b[3]={91,92,93};CHECK(scanf("%d%d%d",&a[0],&a[1],&a[2])==3);int ok=lp_transform(a,b,3);printf("%d %d %d %d\n",ok,b[0],b[1],b[2]);}
        else {CHECK(0);}
    }
    return 0;
}
int main(int argc,char **argv) {
    CHECK(argc==2);
    if(!strcmp(argv[1],"record"))return record();
    if(!strcmp(argv[1],"transform"))return transform();
    if(!strcmp(argv[1],"matrix"))return matrix();
    if(!strcmp(argv[1],"trace"))return trace();
    if(!strcmp(argv[1],"oracle"))return oracle();
    if(!strcmp(argv[1],"metrics")){printf("{\"record_bytes\":%zu,\"int_min\":%d,\"int_max\":%d,\"array_limit\":16}\n",sizeof(struct lp_value),INT_MIN,INT_MAX);return 0;}
    return 2;
}
