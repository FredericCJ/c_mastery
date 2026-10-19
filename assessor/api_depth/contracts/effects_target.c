#include "evolution_v1.h"
static volatile unsigned observed;
static void record(void *context,const unsigned char *data,size_t size) {
 (void)context;unsigned value=0;for(size_t i=0;i<size;++i)value+=data[i];
 observed=value;
}
void _start(void) {
 static const unsigned char input[3]={'A','B','C'};size_t written;
 (void)ac_transform_v1(input,3,record,0,&written);
 for(;;){}
}
