#include "dfa_cases.h"
#include <stdio.h>
#include <string.h>
int main(void){char line[300];struct mm_workspace w;
    while(fgets(line,sizeof line,stdin)) {
        struct mm_dfa d=mm_example();struct mm_tokens out;struct mm_cost c;unsigned err=999,n=(unsigned)strcspn(line+2,"\r\n"),mode=line[0]=='1';
        memset(&w,0xff,sizeof w);int rc=mm_scan(&d,(unsigned char *)line+2,n,mode,256,&w,&out,&c,&err);
        printf("%d %u",rc,rc?err:out.count);
        if(!rc)for(unsigned i=0;i<out.count;++i)printf(" %u:%u:%u",out.token[i].begin,out.token[i].end,out.token[i].rule);
        putchar('\n');
    }return ferror(stdin)?1:0;
}
