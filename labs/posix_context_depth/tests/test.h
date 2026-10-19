#ifndef PC_TEST_H
#define PC_TEST_H
#include <stdio.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"PC_ASSERT:%s:%d:%s\n",__FILE__,__LINE__,#x);exit(41);}}while(0)
#endif
