#include "dispatch.h"
#include <stddef.h>
#include <stdint.h>
extern int abi_write(const void *,unsigned);
static int measure(const void *p,int *out){*out=*(const int *)p;return DS_OK;}
static int failed(const void *p,int *out){(void)p;*out=123;return DS_FAILED;}
int abi_main(void){static const struct ds_ops good={measure},bad={failed};uint32_t words[10]={sizeof(int),sizeof(long),sizeof(void *),sizeof(struct ds_view),_Alignof(struct ds_view),sizeof(struct ds_box),_Alignof(struct ds_box),offsetof(struct ds_box,value),0,0};int checksum=0;for(int i=-16;i<=16;++i){struct ds_box box={&good,i};int out=91;if(ds_measure((struct ds_view){&i,&good},&out)!=DS_OK||out!=i)return 2;if(ds_embedded(&box,&out)!=DS_OK||out!=i)return 3;checksum+=out;box.ops=&bad;out=91;if(ds_embedded(&box,&out)!=DS_FAILED||out!=91)return 4;if(ds_measure((struct ds_view){&i,&bad},&out)!=DS_FAILED||out!=91)return 5;++words[9];}int input[3]={1,2,3},out=91;if(ds_sum(input,3,0,&out)!=DS_OK||out!=6)return 6;words[8]=(uint32_t)(checksum+out);return abi_write(words,sizeof words)==(int)sizeof words?0:7;}
