#include "build_cache.h"
#include <stdio.h>
#include <string.h>
static int read_input(const char *path,struct mpb_bytes *out){
    FILE *f=fopen(path,"rb");int ch;size_t n=0;if(!f)return 0;
    while((ch=fgetc(f))!=EOF){if(n==8){fclose(f);return 0;}out->bytes[n++]=(unsigned char)ch;}
    if(ferror(f)){fclose(f);return 0;}if(fclose(f))return 0;out->length=n;return 1;
}
static void text_input(struct mpb_bytes *out,const char *text){out->length=strlen(text);memcpy(out->bytes,text,out->length);}
int main(int argc,char **argv){
    struct mpb_node node={0};struct mpb_plan plan;FILE *f;
    if(argc!=5||strlen(argv[3])>8)return 2;
    if(!read_input(argv[1],&node.inputs[MPB_SOURCE])||!read_input(argv[2],&node.inputs[MPB_SCHEMA]))return 3;
    node.revision=7;text_input(&node.inputs[MPB_TOOL],"gen1");text_input(&node.inputs[MPB_CONFIG],argv[3]);text_input(&node.inputs[MPB_ENVIRONMENT],"fixed");text_input(&node.inputs[MPB_INCLUDE],"inc1");
    if(mpb_plan(&node,1,0,&plan)!=MPB_OK)return 4;
    f=fopen(argv[4],"wb");if(!f)return 5;
    if(fwrite(plan.entries[0].artifact,1,plan.entries[0].artifact_length,f)!=plan.entries[0].artifact_length){fclose(f);return 6;}
    return fclose(f)?7:0;
}
