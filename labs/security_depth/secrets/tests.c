#include "secrets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do { if(!(x)){fprintf(stderr,"SS_ASSERT:%s\n",#x);return 41;} } while(0)
static int compare(void) {
    const unsigned char token[4]={1,2,3,4};
    const unsigned char candidates[3][4]={{9,2,3,4},{1,2,3,9},{1,2,3,4}};
    unsigned expected[3]={1,4,4}; struct ss_compare out;
    for(unsigned k=0;k<3;++k) {
        CHECK(ss_compare(token,candidates[k],4,1,&out)==0);
        CHECK(out.work==expected[k] && out.equal==(k==2));
        CHECK(ss_compare(token,candidates[k],4,0,&out)==0);
        CHECK(out.work==4 && out.equal==(k==2));
    }
    for(size_t n=0;n<=8;++n) if(n!=4) {
        out=(struct ss_compare){77,88};CHECK(ss_compare(token,token,n,0,&out)==-1);
        CHECK(out.equal==77 && out.work==88);
    }
    for(unsigned k=0;k<4;++k) for(unsigned v=0;v<256;++v) {
        unsigned char c[4]={1,2,3,4};c[k]=(unsigned char)v;
        CHECK(ss_compare(token,c,4,0,&out)==0 && out.work==4 && out.equal==(v==token[k]));
    }
    puts("OBS work early1,4,4 fixed4,4,4 mutations1024 public-length-rejection");return 0;
}
static int lifetime(void) {
    unsigned char original[4]={1,2,3,4},temporary[4]={1,2,3,4};
    unsigned char unmanaged[4]={1,2,3,4};struct ss_pool p={{0},0};
    CHECK(ss_acquire(&p,original)==0 && p.live==1);
    struct ss_pool saved=p;CHECK(ss_acquire(&p,original)==-1 && p.live==saved.live && !memcmp(p.bytes,saved.bytes,4));
    unsigned char *allocated=malloc(4);CHECK(allocated!=NULL);
    for(unsigned i=0;i<4;++i) allocated[i]=original[i];
    ss_wipe(temporary,4);ss_wipe(allocated,4);
    for(unsigned i=0;i<4;++i) CHECK(temporary[i]==0 && allocated[i]==0);
    free(allocated); /* Never inspect storage after deallocation. */
    CHECK(ss_release(&p)==0 && p.live==0);
    for(unsigned i=0;i<4;++i) CHECK(p.bytes[i]==0);
    CHECK(ss_release(&p)==-1);
    for(unsigned i=0;i<4;++i) CHECK(original[i]==i+1 && unmanaged[i]==i+1);
    CHECK(ss_dead_store(original)==10 && ss_explicit_store(original)==10);
    /* Only status labels are logged: no byte pointer crosses the log boundary. */
    puts("OBS lifetime pool-temp-allocated-cleared original-unmanaged-remain log=status-only");return 0;
}
static int addresses(void) {
    unsigned char token[4]={1,2,3,4},first[4]={9,2,3,4},table[256];unsigned a[8],b[8];
    for(unsigned i=0;i<256;++i)table[i]=(unsigned char)(i^165u);
    CHECK(ss_indexed(token,first,table,a)==0);
    CHECK(ss_indexed(token,token,table,b)==1);
    for(unsigned i=0;i<4;++i)CHECK(a[2*i]==token[i] && b[2*i]==token[i]);
    CHECK(a[1]==9 && b[1]==1);
    puts("OBS addresses eight-accesses token-indexed lookup distinguishes input");return 0;
}
int main(int argc,char **argv){if(argc!=2)return 2;if(!strcmp(argv[1],"work"))return compare();if(!strcmp(argv[1],"lifetime"))return lifetime();if(!strcmp(argv[1],"addresses"))return addresses();return 2;}
