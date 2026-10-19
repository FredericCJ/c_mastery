#include "registration.h"
#include <stdio.h>
int main(void) {
    char word[16];
    while (fgets(word,sizeof word,stdin)) {
        MfrHub h;if (mfr_init(&h,2,2)!=MSG_OK) return 1;
        unsigned k=99,v=101;
        for (size_t i=0;word[i] && word[i]!='\n';i++) {
            MsgStatus s=MSG_INVALID;
            switch (word[i]) {
            case '0':s=mfr_send(&h,0,10+(unsigned)i);break;
            case '1':s=mfr_send(&h,1,20+(unsigned)i);break;
            case '2':s=mfr_select(&h,3,false,&k,&v);break;
            case '3':s=mfr_select(&h,1,false,&k,&v);break;
            case '4':s=mfr_close(&h,0);break;
            case '5':s=mfr_close(&h,1);break;
            case '6':mfr_cancel(&h);s=MSG_OK;break;
            default:return 2;
            }
            printf("%u,%u,%u;",(unsigned)s,k,v);
        }
        putchar('\n');if (mfr_destroy(&h)!=MSG_OK) return 3;
    }
    return 0;
}
