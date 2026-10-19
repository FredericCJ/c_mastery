/* Hosted verification driver. stdio and conversion are not component imports. */
#include "codec.h"
#include <stdio.h>
#include <string.h>
static int digit(char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;return -1;}
int main(void)
{
    char op[16],text[4096];uint8_t input[2048],out[2048];
    while(scanf("%15s %4095s",op,text)==2){
        size_t n=0,used=999;int status=CC_INVALID;
        if(strcmp(text,"-")!=0){
            size_t len=strlen(text);if(len%2)return 2;
            for(size_t i=0;i<len;i+=2){int a=digit(text[i]),b=digit(text[i+1]);if(a<0||b<0)return 2;input[n++]=(uint8_t)(16*a+b);}
        }
        if(strcmp(op,"he")==0)status=cc_hencode(input,n,out,sizeof out,&used);
        else if(strcmp(op,"hd")==0)status=cc_hdecode(input,n,out,sizeof out,&used);
        else if(strcmp(op,"le")==0)status=cc_lzencode(input,n,out,sizeof out,&used);
        else if(strcmp(op,"lo")==0)status=cc_lzoptimal(input,n,out,sizeof out,&used);
        else if(strcmp(op,"ld")==0)status=cc_lzdecode(input,n,out,sizeof out,&used);
        else if(strcmp(op,"ce")==0)status=cc_compress(input,n,out,sizeof out,&used);
        else if(strcmp(op,"co")==0)status=cc_compress_optimal(input,n,out,sizeof out,&used);
        else if(strcmp(op,"cd")==0)status=cc_expand(input,n,out,sizeof out,&used);
        else if(strcmp(op,"book")==0&&n==16){CcBook b;status=cc_book(input,&b);if(status==CC_OK){used=48;for(unsigned i=0;i<16;i++){out[3*i]=b.length[i];out[3*i+1]=(uint8_t)(b.code[i]>>8);out[3*i+2]=(uint8_t)b.code[i];}}}
        else if(strcmp(op,"lengths")==0&&n==16){uint16_t f[16];for(unsigned i=0;i<16;i++)f[i]=input[i];status=cc_lengths(f,out);if(status==CC_OK)used=16;}
        printf("%d ",status);if(status==CC_OK){for(size_t i=0;i<used;i++)printf("%02x",out[i]);}else printf("-");puts("");
    }
    return ferror(stdin)?2:0;
}
