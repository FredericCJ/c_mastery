#define _POSIX_C_SOURCE 200809L
#include "effects.h"
#include <errno.h>
#include <locale.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"EFFECTS_STATE_ASSERT:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static volatile sig_atomic_t pending;
static void handler(int signal_number){(void)signal_number;pending=1;}
static const unsigned char*flawed_parse(unsigned char c){static unsigned char scratch[2];scratch[0]=c;scratch[1]=0;return scratch;}
int main(void){
 const unsigned char*a=flawed_parse('a');const unsigned char*b=flawed_parse('b');CHECK(a==b&&a[0]=='b');
 CHECK(signal(SIGUSR1,handler)!=SIG_ERR&&raise(SIGUSR1)==0&&pending==1);
 errno=E2BIG;int saved=errno;FILE*f=tmpfile();CHECK(f);CHECK(fputs("ab",f)>=0&&ftell(f)==2&&fseek(f,0,SEEK_SET)==0&&fgetc(f)=='a'&&ftell(f)==1);CHECK(fclose(f)==0);
 CHECK(setlocale(LC_ALL,"C"));char before[64],after[64];CHECK(snprintf(before,sizeof before,"%.2f",12.5)>0);
 const char*selected=setlocale(LC_NUMERIC,"C.utf8");if(!selected)selected=setlocale(LC_NUMERIC,"C.UTF-8");CHECK(selected);
 char selected_name[64];CHECK(strlen(selected)<sizeof selected_name);strcpy(selected_name,selected);
 CHECK(snprintf(after,sizeof after,"%.2f",12.5)>0&&strcmp(before,after)==0);
 srand(7);int r=rand();srand(7);CHECK(rand()==r);struct ef_parser first={{0},0,7,1,0},second={{0},0,7,1,0};CHECK(ef_random(&first)==ef_random(&second));
 struct {size_t cursor;uint32_t seed,generation;const char*locale_policy;} snapshot={first.cursor,first.seed,first.generation,"C"};
 struct ef_view view={0};CHECK(ef_parse(&first,(const unsigned char*)"replay",6,&view)==EF_OK);unsigned char expected[6];memcpy(expected,view.data,6);
 CHECK(ef_parser_reset(&first,snapshot.seed)==EF_BUSY&&ef_view_release(&view)==EF_OK);
 CHECK(ef_parser_reset(&first,snapshot.seed)==EF_OK&&first.cursor==snapshot.cursor&&first.generation==snapshot.generation+1);
 CHECK(setlocale(LC_NUMERIC,snapshot.locale_policy)&&ef_parse(&first,(const unsigned char*)"replay",6,&view)==EF_OK&&memcmp(expected,view.data,6)==0&&ef_view_release(&view)==EF_OK);
 /* Individually synchronized read/write operations cannot make this sequence
  * atomic: both actors read0 before either publishes1. This serial trace
  * witnesses a lost update without executing a C data race. */
 unsigned shared=0,left=shared,right=shared;shared=left+1;shared=right+1;CHECK(shared==1);
 printf("{\"static_scratch_overwrite\":true,\"signal_flag\":%d,\"saved_errno\":%d,\"locale\":\"%s\",\"equal_numeric_output\":true,\"lost_update_model\":%u}\n",(int)pending,saved,selected_name,shared);return 0;
}
