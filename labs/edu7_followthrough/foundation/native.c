#include <stdlib.h>
int ef_tests(unsigned);
int main(int argc,char **argv){return argc==2?ef_tests((unsigned)strtoul(argv[1],0,10)):41;}
