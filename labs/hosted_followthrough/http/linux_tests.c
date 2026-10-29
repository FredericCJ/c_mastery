#define main hf_server_program
#ifndef HF_SERVER_SOURCE
#define HF_SERVER_SOURCE "server_template.c"
#endif
#include HF_SERVER_SOURCE
#undef main
#include <dirent.h>
#define CHECK(x) do {if(!(x)){fprintf(stderr,"LINUX_CHECK:%d:%s\n",__LINE__,#x);return 1;}}while(0)
static unsigned descriptors(void){unsigned n=0;DIR *d=opendir("/proc/self/fd");if(!d)return 9999;while(readdir(d))n++;closedir(d);return n;}
static int pair(int *a,int *b) {
  struct sockaddr_in addr={0};int listener=-1;
  if(listener_init(0,0,&listener,&addr))return -1;
  int client=socket(AF_INET,SOCK_STREAM,0);if(client<0){close(listener);return -1;}
  int small=256;setsockopt(client,SOL_SOCKET,SO_RCVBUF,&small,sizeof small);
  if(connect(client,(struct sockaddr *)&addr,sizeof addr)){close(client);close(listener);return -1;}
  struct pollfd ready={listener,POLLIN,0};
  if(poll(&ready,1,1000)!=1){close(client);close(listener);return -1;}
  int server=accept(listener,0,0);close(listener);if(server<0){close(client);return -1;}
  if(flags(server)){close(client);close(server);return -1;}
  *a=server;*b=client;return 0;
}
int main(int argc,char **argv) {
  unsigned before=descriptors();
  if(argc==2 && !strcmp(argv[1],"blocked-output")) {
    int a,b;CHECK(!pair(&a,&b));int small=1024;CHECK(!setsockopt(a,SOL_SOCKET,SO_SNDBUF,&small,sizeof small));
    unsigned char padding[4096]={0};size_t total=0;unsigned waits=0;uint64_t start=now_ms();
    /* Bounded owned TCP pair, no reader, and no remote or third-party endpoint.
       Fill only this socket's queue before exercising the flow's output path. */
    while(total<2097152 && now_ms()-start<3000){ssize_t n=send(a,padding,sizeof padding,MSG_NOSIGNAL);if(n>0)total+=(size_t)n;else if(errno==EAGAIN){waits++;struct timespec nap={0,10000000};nanosleep(&nap,0);}else CHECK(0);}
    CHECK(waits && total<2097152);struct hf_flow flow;CHECK(!hf_init(&flow,2,"localhost:8080",now_ms()));size_t used;
    const char req[]="GET /health HTTP/1.1\r\nHost: localhost:8080\r\n\r\n";start=now_ms();CHECK(!hf_feed(&flow,req,sizeof req-1,&used,start,"localhost:8080",0));
    CHECK(hf_write(&flow,write_socket,&a,start)==0 && flow.output[flow.head].offset==0);
    while(now_ms()-start<15000){struct timespec nap={0,10000000};nanosleep(&nap,0);}
    CHECK(hf_tick(&flow,now_ms(),0)==3 && flow.aborted && !flow.draining);close(a);close(b);
    CHECK(descriptors()==before);printf("LINUX_PASS:actual-zero-read-peer:prefill=%zu:EAGAIN=%u:output-timeout-ms=%llu:offset0:abort-not-drain\n",total,waits,(unsigned long long)(now_ms()-start));return 0;
  }
  for(unsigned fault=1;fault<=5;fault++){int fd=-77;struct sockaddr_in a;memset(&a,0xa5,sizeof a);struct sockaddr_in old=a;CHECK(listener_init(0,fault,&fd,&a)==-1 && fd==-77 && !memcmp(&a,&old,sizeof a) && descriptors()==before);}
  int listener=-1;struct sockaddr_in addr={0};CHECK(!listener_init(0,0,&listener,&addr));CHECK((fcntl(listener,F_GETFL)&O_NONBLOCK) && (fcntl(listener,F_GETFD)&FD_CLOEXEC));
  CHECK(accept(listener,0,0)==-1 && (errno==EAGAIN || errno==EWOULDBLOCK));close(listener);
  int a,b;CHECK(!pair(&a,&b));CHECK((fcntl(a,F_GETFL)&O_NONBLOCK) && (fcntl(a,F_GETFD)&FD_CLOEXEC));
  const char bytes[]="abcde";CHECK(send(b,bytes,5,0)==5 && !shutdown(b,SHUT_WR));
  unsigned char got[8];size_t n=0;unsigned wouldblock=0;for(unsigned attempts=0;attempts<50;attempts++){ssize_t z=recv(a,got+n,1,0);if(z>0)n+=(size_t)z;else if(z==0)break;else {CHECK(errno==EAGAIN || errno==EINTR);wouldblock++;struct pollfd f={a,POLLIN,0};poll(&f,1,20);}}
  CHECK(n==5 && !memcmp(got,bytes,5));CHECK(recv(a,got,sizeof got,0)==0);close(a);close(b);CHECK(descriptors()==before);
  printf("LINUX_PASS:5-initializer-failures:unchanged-publication:fd-ledger=%u:accept-EAGAIN:flags:buffered5-before-EOF:waits=%u\n",before,wouldblock);return 0;
}
