#include "tdigest.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"TD_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static double absolute(double x){return x<0?-x:x;}
int main(void){
    Td a,b,old;double values[64],q=19,cdf=19,total=0,weighted=0;unsigned i,j;
    CHECK(td_init(&a,16)==TD_OK && td_valid(&a));
    CHECK(td_quantile(&a,.5,&q)==TD_INVALID && q==19);
    CHECK(td_add(&a,0,0)==TD_OK);
    values[0]=0;values[1]=10;CHECK(td_add(&a,values,2)==TD_OK);
    CHECK(td_quantile(&a,0,&q)==TD_OK&&q==0);CHECK(td_quantile(&a,.5,&q)==TD_OK&&q==0);
    CHECK(td_cdf(&a,5,&cdf)==TD_OK&&cdf==.5);CHECK(td_quantile(&a,1,&q)==TD_OK&&q==10);
    old=a;values[0]=NAN;CHECK(td_add(&a,values,1)==TD_INVALID&&memcmp(&a,&old,sizeof a)==0);
    values[0]=INFINITY;CHECK(td_add(&a,values,1)==TD_INVALID&&memcmp(&a,&old,sizeof a)==0);
    q=19;CHECK(td_quantile(&a,NAN,&q)==TD_INVALID&&q==19);
    CHECK(td_init(&a,32)==TD_OK);CHECK(td_init(&b,32)==TD_OK);
    for(j=0;j<16;j++){
        for(i=0;i<64;i++){double x=(double)(j*64+i);values[i]=x*x;total+=values[i];}
        CHECK(td_add(j%2?&a:&b,values,64)==TD_OK);
    }
    CHECK(td_merge(&a,&b)==TD_OK&&td_valid(&a)&&a.count==1024);
    for(i=0;i<a.used;i++)weighted+=a.centroids[i].mean*(double)a.centroids[i].weight;
    CHECK(absolute(weighted-total)<1e-5);
    CHECK(a.used<128&&a.centroids[0].weight==1&&a.centroids[a.used-1].weight==1);
    CHECK(td_quantile(&a,0,&q)==TD_OK&&q==0);CHECK(td_quantile(&a,1,&q)==TD_OK&&q==1023.*1023.);
    old=a;CHECK(td_recompress(&a,33)==TD_INVALID&&memcmp(&a,&old,sizeof a)==0);
    CHECK(td_recompress(&a,8)==TD_OK&&td_valid(&a));
    old=b;CHECK(td_merge(&b,&a)==TD_INVALID&&memcmp(&b,&old,sizeof b)==0);
    CHECK(td_merge(&a,&a)==TD_OK&&a.count==2048&&td_valid(&a));
    while(a.count<65536)CHECK(td_merge(&a,&a)==TD_OK);
    old=a;CHECK(td_merge(&a,&a)==TD_CAPACITY&&memcmp(&a,&old,sizeof a)==0);
    CHECK(td_init(&a,8)==TD_OK);for(i=0;i<64;i++)values[i]=7;
    CHECK(td_add(&a,values,64)==TD_OK);CHECK(td_quantile(&a,.25,&q)==TD_OK&&q==7);
    CHECK(td_cdf(&a,7,&cdf)==TD_OK&&cdf==1);CHECK(td_cdf(&a,6,&cdf)==TD_OK&&cdf==0);
    puts("tdigest: singleton tails, weighted mean, transactional rejection, two-level merge and finite bounds checked");return 0;
}
