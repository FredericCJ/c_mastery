#include "flow.h"
#include <stdio.h>
#include <string.h>
static int digit(int c){return c>='0' && c<='9'?c-'0':c>='a' && c<='f'?c-'a'+10:-1;}
static void hex(const void *v,size_t n){const unsigned char *s=v;for(size_t i=0;i<n;i++)printf("%02x",s[i]);}
int main(void) {
  char line[50000],id[80],authority[128],encoded[40001];unsigned char bytes[20000],output[1600];
  while(fgets(line,sizeof line,stdin)) {
    size_t split,second;unsigned eof;int got=sscanf(line,"%79s %zu %zu %u %127s %40000s",id,&split,&second,&eof,authority,encoded);
    if(got!=6 || strlen(encoded)%2)return 2;
    size_t n=strlen(encoded)/2;if(n>sizeof bytes || split>second || second>n)return 2;
    for(size_t i=0;i<n;i++){int a=digit(encoded[2*i]),b=digit(encoded[2*i+1]);if(a<0 || b<0)return 2;bytes[i]=(unsigned char)(a*16+b);}
    HttpParser p;http_init(&p,authority);size_t total=0,first=0,take=0;int r=http_feed(&p,bytes,split,&first),prefix=r;total=first;
    while(total<n && (r==HTTP_NEED || r==HTTP_CONTINUE)) {size_t end=total<second?second:n;r=http_feed(&p,bytes+total,end-total,&take);total+=take;if(!take)break;}
    if(eof && r!=HTTP_DONE && r!=HTTP_ERROR)r=http_eof(&p);
    size_t z=0;int sr=(r==HTTP_DONE || r==HTTP_ERROR)?hf_response(&p.request,eof==2?0:"Sun, 01 Jan 2023 00:00:00 GMT",0,0,0,output,sizeof output,&z):-2;
    printf("{\"id\":\"%s\",\"result\":%d,\"prefix_result\":%d,\"prefix_used\":%zu,\"used\":%zu,\"status\":%d,\"state\":%u,\"body_length\":%zu,\"body_hex\":\"",id,r,prefix,first,total,p.request.status,p.state,p.request.body_length);hex(p.request.body,p.request.body_length);
    printf("\",\"authority\":\"%s\",\"serializer\":%d,\"response_hex\":\"",p.request.authority,sr);if(!sr)hex(output,z);puts("\"}");
  }
  return ferror(stdin)?2:0;
}
