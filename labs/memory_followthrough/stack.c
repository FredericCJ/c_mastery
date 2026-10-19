#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
extern void mw_stack_probe(unsigned long out[2]);
static volatile unsigned sink;
__attribute__((noinline)) static unsigned descend(unsigned depth){volatile unsigned char frame[4096];for(unsigned i=0;i<sizeof frame;i++)frame[i]=(unsigned char)(depth+i);if(depth) sink=descend(depth-1);return sink+frame[0]+frame[4095];}
__attribute__((noinline)) static unsigned recursive_tree(unsigned depth){volatile unsigned held=depth;if(!held)return 1;unsigned left=recursive_tree(held-1);return left+recursive_tree(held-1);}
int main(int argc,char **argv){if(argc==1){if(recursive_tree(6)!=64)return 5;puts("MW:recursive-tree:depth6:leaves64");unsigned long out[2]={0};mw_stack_probe(out);printf("MW:stack-pattern:changed%lu:reserved%lu:restored8:untouched32\n",out[0],out[1]);return out[0]!=24||out[1]!=64;}if(argc!=2)return 2;unsigned depth=(unsigned)strtoul(argv[1],NULL,10);if(depth>128)return 2;struct rlimit stack,core;if(getrlimit(RLIMIT_STACK,&stack)||getrlimit(RLIMIT_CORE,&core))return 3;if(stack.rlim_cur>65536||core.rlim_cur!=0)return 4;printf("SETUP:stack-limit%lu:core%lu:bounded-depth%u\nENTRY:stack\n",(unsigned long)stack.rlim_cur,(unsigned long)core.rlim_cur,depth);fflush(stdout);sink=descend(depth);puts("EXIT:stack");return 0;}
