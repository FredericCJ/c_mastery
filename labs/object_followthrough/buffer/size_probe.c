#include "buffer.h"
#include <stdio.h>
int main(void)
{
    size_t prefixes[]={0,16,sizeof(ObFlex),SIZE_MAX},widths[]={1,4,8,SIZE_MAX},counts[]={0,1,3,100,SIZE_MAX},limits[]={0,15,255,SIZE_MAX};
    for(size_t p=0;p<4;++p)for(size_t w=0;w<4;++w)for(size_t c=0;c<5;++c)for(size_t l=0;l<4;++l){size_t out=99;int status=ob_size(prefixes[p],widths[w],counts[c],limits[l],&out);printf("%zu,%zu,%zu,%zu,%d,%zu\n",prefixes[p],widths[w],counts[c],limits[l],status,out);}
    return 0;
}
