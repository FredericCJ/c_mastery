#include "variant.h"
#include "flow.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"NV_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
  const char *fields[]={"Content-Length: 5\r\nX-Neutral: ignored\r\n", "Content-Length: 6\r\nTransfer-Encoding: chunked\r\n", "Content-Length : 5\r\n", "Content-Length: 5\r\n folded\r\n"};
  unsigned cases=0;
  for(unsigned k=0;k<4;k++){
    char wire[256];int n=snprintf(wire,sizeof wire,"POST /echo HTTP/1.1\r\nHost: localhost:8080\r\n%s\r\n%s",fields[k],k==1?"0\r\n\r\nSYNTHETIC":"abcde");CHECK(n>0 && (size_t)n<sizeof wire);
    for(size_t split=0;split<=(size_t)n;split++){
      HttpParser p;http_init(&p,"localhost:8080");size_t a=99,b=99;
      int r=http_feed(&p,wire,split,&a);CHECK(a<=split);
      if(r==HTTP_NEED)r=http_feed(&p,wire+split,(size_t)n-split,&b);
      if(k==0){CHECK(r==HTTP_DONE && p.request.status==200 && p.request.body_length==5 && !memcmp(p.request.body,"abcde",5));}
      else {CHECK(r==HTTP_ERROR && p.request.status==400 && p.request.close);size_t used=99;CHECK(http_feed(&p,"GET /",5,&used)==HTTP_ERROR && used==0);}
      cases++;
    }
  }
  /* A real successor serializer consumes the new Date representation. */
  char date[30];size_t length=0;NVStamp stamp={1,1,NV_POSITIVE_LEAP,17166,86400};CHECK(!nv_stamp(stamp,17166,date,30,&length));
  HttpRequest r={0};r.status=200;strcpy(r.target,"/health");unsigned char response[1600];size_t n;
  CHECK(!hf_response(&r,date,0,0,0,response,sizeof response-1,&n));response[n]=0;CHECK(strstr((char *)response,"Date: Sat, 31 Dec 2016 23:59:60 GMT\r\n"));
  stamp.trusted=0;CHECK(!nv_stamp(stamp,UINT32_MAX,date,30,&length) && !length);
  CHECK(!hf_response(&r,0,0,0,0,response,sizeof response-1,&n));response[n]=0;CHECK(!strstr((char *)response,"Date:"));
  printf("NV_OK:framing:%u:all-splits,neutral,fold,colon,conflict-close,leap-and-omitted-Date\n",cases);return 0;
}
