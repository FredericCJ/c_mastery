#include <stdio.h>
#include <string.h>
#include <errno.h>
static char output[64];static unsigned used;
static int put(char c,FILE*f){(void)f;if(used>=sizeof output)return EOF;output[used++]=c;return(unsigned char)c;}
static int get(FILE*f){(void)f;return EOF;}
static FILE console=FDEV_SETUP_STREAM(put,get,NULL,_FDEV_SETUP_RW);
FILE*const stdin=&console;FILE*const stdout=&console;FILE*const stderr=&console;
int main(void){if(printf("value=%d",17)!=8||used!=8||memcmp(output,"value=17",8))return 1;errno=0;if(fseek(stdout,0,SEEK_SET)!=-1||errno!=ESPIPE)return 2;return 0;}
