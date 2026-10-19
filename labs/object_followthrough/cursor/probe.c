#include "cursor.h"
#include <stdio.h>
static int index_of(const unsigned char *a,const unsigned char *p)
{for(int i=0;i<=8;++i)if(p==a+i)return i;return -1;}
int main(void)
{
    unsigned char a[8]={0};
    for(size_t n=0;n<=8;++n)for(size_t position=0;position<=n;++position){
        for(size_t count=0;count<=10;++count){
            FDCursor c={a,n,position};const unsigned char *p=a+8;
            int ok=oc_take(&c,count,&p)==FD_OK;
            printf("take,%zu,%zu,%zu,%d,%zu,%d\n",n,position,count,ok,c.position,index_of(a,p));
            c=(FDCursor){a,n,position};p=a+8;ok=oc_previous(&c,count,&p)==FD_OK;
            printf("reverse,%zu,%zu,%zu,%d,%zu,%d\n",n,position,count,ok,c.position,index_of(a,p));
        }
        for(size_t offset=0;offset<=10;++offset)for(size_t count=0;count<=10;++count){
            FDCursor c={a,n,position};int ok=oc_subview(&c,offset,count,&c)==FD_OK;
            printf("sub,%zu,%zu,%zu,%zu,%d,%d,%zu,%zu\n",n,position,offset,count,ok,index_of(a,c.base),c.length,c.position);
        }
    }
    return 0;
}
