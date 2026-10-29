#include "writer.h"
#include <stdio.h>
int main(void)
{
    struct cwf_span span={0};const unsigned char bytes[]="ABCDEFGHI";
    if(cwf_span_begin(&span,bytes,9,71)!=0){fputs("CWF_UNFINISHED:span admission\n",stderr);return 41;}
    puts("CWF_ADMISSION_PASS");return 0;
}
