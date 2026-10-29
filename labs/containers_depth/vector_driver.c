#include "containers.h"
#include <stdio.h>
#include <stdlib.h>
struct cd_alloc{unsigned fail,live;};
static int*take(void*v,size_t n){struct cd_alloc*a=v;if(a->fail)return NULL;int*p=malloc(n*sizeof*p);if(p)++a->live;return p;}
static void release(void*v,int*p){struct cd_alloc*a=v;--a->live;free(p);}
int main(void){struct cd_alloc a={0};struct cd_vector vector;if(cd_vector_init(&vector,(struct cd_allocator){&a,take,release})!=CD_OK)return 2;char op;size_t at;int value;unsigned fail;
 while(scanf(" %c %zu %d %u",&op,&at,&value,&fail)==4){a.fail=fail;enum cd_status status=CD_INVALID;int old=0;
  if(op=='z'){cd_vector_destroy(&vector);status=cd_vector_init(&vector,(struct cd_allocator){&a,take,release});}
  else if(op=='i')status=cd_vector_insert(&vector,at,value);
  else if(op=='e')status=cd_vector_erase(&vector,at,&old);
  else if(op=='r')status=cd_vector_reserve(&vector,at);
  else if(op=='s')status=cd_vector_shrink(&vector);
  printf("%d %zu %zu %u",status,vector.length,vector.capacity,a.live);for(size_t i=0;i<vector.length;++i)printf(" %d",vector.data[i]);putchar('\n');
 }return cd_vector_destroy(&vector)==CD_OK&&a.live==0?0:3;
}
