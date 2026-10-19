#include <stdio.h>
#include <string.h>
int main(int argc,char**argv){if(argc!=2)return 2;FILE*f=fopen(argv[1],"w+");if(!f)return 3;int bad=fputs("synthetic",f)<0||fflush(f)||fseek(f,0,SEEK_SET);char out[16];if(!bad)bad=!fgets(out,sizeof out,f)||strcmp(out,"synthetic");if(fclose(f))bad=1;return bad;}
