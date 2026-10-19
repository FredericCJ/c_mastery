/* Original deliberate nontermination control, bounded by parent process timeout. */
#include <stdio.h>
int main(int argc,char **argv){
 if(argc!=2)return 2;
 FILE *f=fopen(argv[1],"rb");if(!f)return 2;int first=fgetc(f);if(fclose(f))return 2;
 if(first==255)for(;;){}
 puts("TF_TIMEOUT_CONTROL:terminated");return 0;
}
