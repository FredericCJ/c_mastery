#include "evolution_v1.h"
static unsigned calls;static unsigned char copy[8];
static void consume(void *context,const unsigned char *data,size_t size) {
 (void)context;++calls;for(size_t i=0;i<size;++i)copy[i]=data[i];
}
int main(void) {
 size_t written=71;const unsigned char input[3]={'A','B','C'};
 if(ac_transform_v1(input,3,consume,0,&written)||written!=3||calls!=1||copy[2]!='c')return 1;
 if(ac_transform_v1(0,0,consume,0,&written)||written||calls!=2)return 2;
 written=71;
 if(ac_transform_v1(0,1,consume,0,&written)!=1||written!=71||calls!=2)return 3;
 return 0;
}
