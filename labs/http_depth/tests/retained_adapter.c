#include "com_http.h"
#include "http_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"HTTP_ADAPTER:%d\n",__LINE__);return 1;}}while(0)
int main(void){
  const char *values[]={"0","1024","1025","4294967295","4294967296","1025x"};
  const int expected[]={200,200,413,413,400,400};
  for(unsigned i=0;i<6;i++)for(unsigned extra=0;extra<2;extra++){
    char wire[300];int n=snprintf(wire,sizeof wire,"POST /echo HTTP/1.1\r\nHost: localhost:8080\r\nContent-Length: %s\r\n%s\r\n",values[i],extra?"Transfer-Encoding: chunked\r\n":"");
    for(size_t split=0;split<=(size_t)n;split++){
      struct http_parser p;size_t used;http_init(&p,"localhost:8080");
      (void)http_feed(&p,wire,split,&used);(void)http_feed(&p,wire+split,(size_t)n-split,&used);
      CHECK(p.request.status==(extra?400:expected[i]));
    }
  }
  struct http_request req={0};strcpy(req.method,"GET");strcpy(req.target,"/health");req.status=200;
  char date[30],out[512];size_t len=0,n=0;CHECK(hd_date(1,19358,0,date,sizeof date,&len)==0);
  CHECK(http_response(&req,date,1,out,sizeof out-1,&n)==0);out[n]=0;
  CHECK(strstr(out,"Date: Sun, 01 Jan 2023 00:00:00 GMT\r\n"));
  puts("retained HTTP composition: fixed-u32 framing all splits, TE precedence, generated UTC Date");return 0;
}
