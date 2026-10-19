#include <string.h>
int main(void) {
    unsigned char token[4]={1,2,3,4};
    if(memset_explicit(token,0,4)!=token)return 1;
    for(unsigned i=0;i<4;++i)if(token[i])return 1;
    return 0;
}
