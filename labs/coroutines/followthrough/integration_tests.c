#define _POSIX_C_SOURCE 200809L
#include "integration_follow.h"
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <limits.h>
#include <fcntl.h>
#define C(x) do{if(!(x)){fprintf(stderr,"CI_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static long long now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))return LLONG_MIN;return (long long)t.tv_sec*1000000000LL+t.tv_nsec;}
struct co_writer{int fd;size_t quota;};
static ptrdiff_t writer(void*v,const unsigned char*b,size_t n){struct co_writer*w=v;if(n>w->quota)n=w->quota;if(!n)return 0;return write(w->fd,b,n);}
static int cursor(void){
 const unsigned char bytes[]={'z','a','c'};struct f_cursor c={bytes,3,1};struct co_parser parser[]={{CO_SEQUENCE,1,2,0},{CO_LITERAL,0,0,'a'},{CO_LITERAL,0,0,'b'}};size_t consumed=91;unsigned steps=92;
 C(ci_parse_cursor(&c,parser,3,20,&consumed,&steps)==CO_NO_MATCH&&c.position==1&&consumed==91);
 parser[2].literal='c';C(ci_parse_cursor(&c,parser,3,20,&consumed,&steps)==CO_OK&&c.position==3&&consumed==2);return 0;
}
static int receive_case(unsigned mode){
 int gates[2][2],data[2],cancel[2];C(pipe(gates[0])==0&&pipe(gates[1])==0&&pipe(data)==0&&pipe(cancel)==0);unsigned char count=mode==5?200:1;
 C(write(gates[0][1],&count,1)==1&&write(gates[1][1],&count,1)==1);int fds[]={gates[0][0],gates[1][0]};
 const char*payload=mode==1?"4":mode==4?"4x":"42";size_t length=strlen(payload);C(write(data[1],payload,length)==(ssize_t)length);C(close(data[1])==0);
 if(mode==3)C(write(cancel[1],"!",1)==1);
 struct sf_stream stream;memset(&stream,0x5a,sizeof stream);struct sf_stream original=stream;struct sf_handle handle={0};int32_t value=91;struct ci_progress progress;
 int r=ci_receive(fds,data[0],cancel[0],mode==2?LLONG_MIN:now()+1000000000LL,&stream,&handle,&value,&progress);
 int expected=mode==0?CO_DONE:mode==1||mode==4?CO_NO_MATCH:mode==5?CO_RANGE:CO_IO;C(r==expected);
 if(!mode){
  C(value==42&&progress.accepted==2&&progress.reads==1&&handle.owner==&stream);
  int output[2];C(pipe(output)==0);struct co_writer w={output[1],2};size_t sent=91;C(sf_pump(handle,writer,&w,1,&sent)==SF_PENDING&&sent==2);
  w.quota=0;C(sf_pump(handle,writer,&w,9,&sent)==SF_PENDING&&sent==2);w.quota=1;C(sf_pump(handle,writer,&w,3,&sent)==SF_DONE&&sent==4);
  C(close(output[1])==0);unsigned char frame[5]={0};C(read(output[0],frame,5)==4&&frame[0]==2&&frame[1]=='4'&&frame[2]=='2'&&frame[3]==102);C(close(output[0])==0);
 }else{C(value==91&&!handle.owner&&memcmp(&stream,&original,sizeof stream)==0);if(mode==1)C(progress.accepted==1&&progress.arms==1&&progress.disarms==1);if(mode==2||mode==3){unsigned char first=0;C(progress.accepted==0&&read(data[0],&first,1)==1&&first=='4');C(progress.port_result==(mode==2?-2:-3));}}
 C(fcntl(data[0],F_GETFD)>=0&&fcntl(cancel[0],F_GETFD)>=0);for(unsigned i=0;i<2;i++){C(close(gates[i][0])==0&&close(gates[i][1])==0&&close(cancel[i])==0);}C(close(data[0])==0);return 0;
}
int main(void){C(cursor()==0);for(unsigned mode=0;mode<6;mode++)C(receive_case(mode)==0);puts("CI_IMPORTS:actual reactor/deadline/cursor/integer/frame-writer sources;6 real local pipe histories");return 0;}
