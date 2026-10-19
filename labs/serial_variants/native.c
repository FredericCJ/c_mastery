#include <stdio.h>
#include <stdlib.h>
void sv_note(const char *s){fputs(s,stdout);}
int sv_test(unsigned);
int main(int argc,char **argv){return argc==2?sv_test((unsigned)strtoul(argv[1],0,10)):41;}
