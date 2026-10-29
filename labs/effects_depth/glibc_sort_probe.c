/* GNU libc-specific observation. __libc_malloc is outside ISO C. */
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <gnu/libc-version.h>
void*__libc_malloc(size_t);void __libc_free(void*);
static unsigned tracing,fail,requests;static size_t requested;
void*malloc(size_t n){if(tracing){++requests;requested=n;if(fail){errno=ENOMEM;return NULL;}}return __libc_malloc(n);}
void free(void*p){__libc_free(p);}
static int cmp(const void*a,const void*b){int x=*(const int*)a,y=*(const int*)b;return(x>y)-(x<y);}
struct ef_record {int key;unsigned char payload[60];};
int main(void){
 int a[300];struct ef_record large[10];unsigned counts[4];size_t sizes[4];
 for(unsigned mode=0;mode<4;++mode){for(unsigned i=0;i<300;++i)a[i]=(int)(300-i);for(unsigned i=0;i<10;++i)large[i].key=(int)(10-i);
 requests=0;requested=0;fail=mode==3;errno=E2BIG;tracing=1;
 if(mode==2)qsort(large,10,sizeof*large,cmp);else qsort(a,mode==0?64:300,sizeof*a,cmp);
 tracing=0;counts[mode]=requests;sizes[mode]=requested;
 if(errno!=E2BIG)return 1;
 if(mode==2){for(unsigned i=1;i<10;++i)if(large[i-1].key>large[i].key)return 2;}else for(unsigned i=1;i<(mode==0?64u:300u);++i)if(a[i-1]>a[i])return 3;
 }
 if(counts[0]!=0||counts[1]!=1||sizes[1]!=1200||counts[2]!=0||counts[3]!=1||sizes[3]!=1200)return 4;
 printf("{\"runtime\":\"%s\",\"observed_requests\":[%u,%u,%u,%u],\"observed_sizes\":[%zu,%zu,%zu,%zu],\"malloc_failure_sorted\":true,\"errno_preserved\":true}\n",gnu_get_libc_version(),counts[0],counts[1],counts[2],counts[3],sizes[0],sizes[1],sizes[2],sizes[3]);return 0;
}
