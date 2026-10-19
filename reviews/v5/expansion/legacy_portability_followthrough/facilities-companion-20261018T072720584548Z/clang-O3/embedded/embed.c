#include <stdio.h>
#include <limits.h>
_Static_assert(CHAR_BIT==8,"octet resource");
static const unsigned char resource[]={
#embed "resource.bin"
};
int main(void){for(unsigned i=0;i<sizeof resource;i++)printf("%02x",resource[i]);puts("");return 0;}
