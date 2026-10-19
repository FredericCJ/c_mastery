#include "algorithms.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ALGORITHM_SORT_ASSERT:%d:%s\n",__LINE__,#x);exit(1);}}while(0)
static void sorted(const ADRecord*p,size_t n,unsigned stable){unsigned seen=0;for(size_t i=0;i<n;++i){CHECK(p[i].label<n&&!(seen&(1u<<p[i].label)));seen|=1u<<p[i].label;if(i){CHECK(p[i-1].key<=p[i].key);if(stable&&p[i-1].key==p[i].key)CHECK(p[i-1].label<p[i].label);}}CHECK(seen==(1u<<n)-1);}
static unsigned cases;
static void sort_cases(void){
 for(unsigned n=0;n<=8;++n){unsigned count=1;for(unsigned i=0;i<n;++i)count*=3;for(unsigned code=0;code<count;++code){ADRecord initial[16],p[16],scratch[16];unsigned value=code;for(unsigned i=0;i<n;++i){initial[i]=(ADRecord){(int32_t)(value%3)-1,i};value/=3;}
  for(unsigned algorithm=0;algorithm<8;++algorithm){memcpy(p,initial,n*sizeof*p);ADWork work={0};enum ADStatus status=AD_FAILED;
   if(algorithm==0)status=ad_insertion(p,n,&work);else if(algorithm==1)status=ad_merge_sort(p,n,scratch,16,&work);else if(algorithm==2)status=ad_heap_sort(p,n,&work);else if(algorithm==3)status=ad_quick_sort(p,n,6,0,&work);else if(algorithm==4)status=ad_quick_sort(p,n,2,1,&work);else if(algorithm==5)status=ad_counting(p,n,scratch,16,&work);else status=ad_radix(p,n,algorithm==6?4:8,scratch,16,&work);
   CHECK(status==AD_OK);sorted(p,n,algorithm==0||algorithm==1||algorithm>=5);for(unsigned i=0;i<n;++i)CHECK(p[i].key==initial[p[i].label].key);CHECK(work.max_depth<=5);
  }++cases;
 }}
}
int main(void){
 int32_t linear_keys[]={4,1,4};size_t linear_index;CHECK(ad_linear(linear_keys,3,4,&linear_index)==AD_OK&&linear_index==0);CHECK(ad_linear(linear_keys,3,9,&linear_index)==AD_OK&&linear_index==3);CHECK(ad_linear(NULL,0,1,&linear_index)==AD_OK&&!linear_index);
 int32_t keys[5]={1,2,2,2,5},query[3]={0,2,6};size_t lower,upper,exact;for(unsigned q=0;q<3;++q){CHECK(ad_bounds(keys,5,query[q],0,&lower,&upper,&exact)==AD_OK);size_t l=0,u=0;while(l<5&&keys[l]<query[q])++l;while(u<5&&keys[u]<=query[q])++u;CHECK(lower==l&&upper==u&&exact==(l<5&&keys[l]==query[q]?l:5));}
 int32_t descending[5]={5,2,2,2,1};CHECK(ad_bounds(descending,5,2,1,&lower,&upper,&exact)==AD_OK&&lower==1&&upper==4&&exact==1);CHECK(ad_bounds(NULL,0,2,0,&lower,&upper,&exact)==AD_OK&&!lower&&!upper&&!exact);
 ADRecord p[16]={{3,0},{1,1},{2,2},{3,3},{0,4}},scratch[16],before[16];int32_t pivot=2;size_t cut;ADWork work={0};CHECK(ad_partition(p,5,&pivot,0,NULL,0,&cut,&work)==AD_OK&&cut==2);for(unsigned i=0;i<5;++i)CHECK((p[i].key<2)==(i<cut));
 ADRecord stable[5]={{3,0},{1,1},{2,2},{3,3},{0,4}};memcpy(before,stable,sizeof stable);CHECK(ad_partition(stable,5,&pivot,1,scratch,4,&cut,&work)==AD_SPACE&&!memcmp(before,stable,sizeof stable));CHECK(ad_partition(stable,5,&pivot,1,scratch,5,&cut,&work)==AD_OK&&cut==2&&stable[0].label==1&&stable[1].label==4&&stable[2].label==0&&stable[3].label==2&&stable[4].label==3);
 ADRecord merge[4]={{1,0},{3,1},{1,2},{2,3}};CHECK(ad_merge(merge,2,4,scratch,4,&work)==AD_OK&&merge[0].label==0&&merge[1].label==2&&merge[2].label==3&&merge[3].label==1);memcpy(before,merge,sizeof merge);CHECK(ad_merge_sort(merge,4,scratch,3,&work)==AD_SPACE&&!memcmp(before,merge,sizeof merge));
 ADRecord three[6]={{5,0},{1,1},{5,2},{0,3},{8,4},{5,5}};CHECK(ad_three_way(three,6,&three[0].key,&lower,&upper,&work)==AD_OK&&lower==2&&upper==5);for(unsigned i=0;i<6;++i)CHECK(i<lower?three[i].key<5:i<upper?three[i].key==5:three[i].key>5);
 ADRecord original[5]={{5,0},{1,1},{5,2},{0,3},{8,4}};int32_t ranks[5]={0,1,5,5,8};for(unsigned k=0;k<5;++k){memcpy(p,original,sizeof original);int32_t value;CHECK(ad_select(p,5,k,0,&value,&lower,&upper,&work)==AD_OK&&value==ranks[k]&&lower<=k&&k<upper);for(unsigned i=0;i<5;++i)CHECK(i<lower?p[i].key<value:i<upper?p[i].key==value:p[i].key>value);}
 memcpy(p,original,sizeof original);int32_t selected=99;CHECK(ad_select(p,5,5,0,&selected,&lower,&upper,&work)==AD_INVALID&&!memcmp(p,original,sizeof original)&&selected==99&&ad_select(p,5,0,1,&selected,&lower,&upper,&work)==AD_INVALID&&ad_select(NULL,0,0,0,&selected,&lower,&upper,&work)==AD_INVALID);CHECK(ad_select(p,5,5,1,&selected,&lower,&upper,&work)==AD_OK&&selected==8);
 ADRecord equal[3]={{4,0},{4,1},{4,2}};CHECK(ad_select(equal,3,1,0,&selected,&lower,&upper,&work)==AD_OK&&lower==0&&upper==3);
 for(unsigned style=0;style<4;++style)for(unsigned budget=0;budget<8;++budget){for(unsigned i=0;i<16;++i)p[i]=(ADRecord){style==0?(int)i:style==1?15-(int)i:style==2?4:(int)((i%2)?i:16-i),i};work=(ADWork){0};CHECK(ad_quick_sort(p,16,budget,style%2,&work)==AD_OK&&work.max_depth<=5);sorted(p,16,0);if(!budget)CHECK(work.fallbacks==1);}
 ADRecord limits[5]={{INT32_MIN,0},{INT32_MAX,1},{0,2},{-1,3},{INT32_MIN,4}};CHECK(ad_radix(limits,5,8,scratch,5,&work)==AD_OK);sorted(limits,5,1);CHECK(limits[0].key==INT32_MIN&&limits[1].key==INT32_MIN&&limits[4].key==INT32_MAX);
 ADWork increasing={0},decreasing={0};for(unsigned i=0;i<8;++i)p[i]=(ADRecord){(int32_t)i,i};CHECK(ad_insertion(p,8,&increasing)==AD_OK);for(unsigned i=0;i<8;++i)p[i]=(ADRecord){7-(int32_t)i,i};CHECK(ad_insertion(p,8,&decreasing)==AD_OK&&increasing.comparisons==7&&decreasing.comparisons==28);
 sort_cases();printf("ALGORITHM_SORT_PASS exhaustive_ternary_cases=%u implementations=8 ordered_comparisons=%llu reverse_comparisons=%llu\n",cases,(unsigned long long)increasing.comparisons,(unsigned long long)decreasing.comparisons);return 0;
}
