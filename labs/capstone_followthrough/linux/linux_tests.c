#define _GNU_SOURCE
#include "linux.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include <stdlib.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CAP_LINUX:%s:%d\n",#x,__LINE__);return 1;}}while(0)
static int fixed_pipe(int *writer){int p[2];if(pipe2(p,O_NONBLOCK|O_CLOEXEC))return -1;int w=fcntl(p[1],F_DUPFD_CLOEXEC,10);if(w<0)return -1;if(close(p[1]))return -1;
    if(p[0]!=5){if(dup2(p[0],5)!=5 || close(p[0]))return -1;}*writer=w;return 5;}
static int fairness(void){
    for(int edge=0;edge<2;++edge){int ep=epoll_create1(EPOLL_CLOEXEC),w;CHECK(ep==3);CHECK(fixed_pipe(&w)==5);struct cs_service s;struct cs_linux a;CHECK(cs_init(&s,3,100)==CS_OK);csl_init(&a,&s,ep);CHECK(!csl_open(&a,0,5,edge));
        CHECK(write(w,"0123456789",10)==10);struct epoll_event e;CHECK(epoll_wait(ep,&e,1,1000)==1 && csl_event(&a,e.data.u64)==CS_OK);
        struct cs_turn first=csl_turn(&a,0,4,1);CHECK(first.bytes==4 && first.status==CS_PENDING && a.session[0].ready);
        int observed=epoll_wait(ep,&e,1,0);CHECK(observed==(edge?0:1));
        struct cs_turn second=csl_turn(&a,0,4,1),third=csl_turn(&a,0,4,1);CHECK(second.bytes==4 && second.status==CS_PENDING && third.bytes==2 && third.status==CS_MORE && !a.session[0].ready);
        uint64_t stale=csl_token(&a,0);CHECK(!csl_close(&a,0) && !close(w));CHECK(fixed_pipe(&w)==5 && !csl_open(&a,0,5,edge));
        CHECK(a.session[0].generation==8 && csl_event(&a,stale)==CS_STALE && !a.session[0].ready && !s.accepted);
        CHECK(write(w,"A",1)==1 && epoll_wait(ep,&e,1,1000)==1 && csl_event(&a,e.data.u64)==CS_OK);CHECK(csl_turn(&a,0,4,1).bytes==1);CHECK(!csl_close(&a,0) && !close(w) && !close(ep));
    }puts("LT+ET=4,4,2,EAGAIN fd5/g7->fd5/g8 stale-rejected-before-read");return 0;}
static int duplicate(void){int ep=epoll_create1(EPOLL_CLOEXEC),p[2];CHECK(ep>=0 && !pipe2(p,O_NONBLOCK|O_CLOEXEC));int duplicate=dup(p[0]);CHECK(duplicate>=0);
    struct epoll_event e={.events=EPOLLIN|EPOLLET,.data.u64=7};CHECK(!epoll_ctl(ep,EPOLL_CTL_ADD,p[0],&e));e.data.u64=8;CHECK(!epoll_ctl(ep,EPOLL_CTL_ADD,duplicate,&e));
    CHECK(!close(p[0]) && write(p[1],"Q",1)==1);struct epoll_event events[2];int n=epoll_wait(ep,events,2,1000);CHECK(n==2);unsigned mask=0;for(int i=0;i<n;++i)mask|=events[i].data.u64==7?1u:events[i].data.u64==8?2u:0u;CHECK(mask==3);
    char byte;CHECK(read(duplicate,&byte,1)==1 && byte=='Q');/* closed integer's registration survives while this OFD has a reference */
    CHECK(!epoll_ctl(ep,EPOLL_CTL_DEL,duplicate,NULL) && !close(duplicate) && !close(p[1]));CHECK(epoll_wait(ep,events,2,0)==0 && !close(ep));
    puts("dup-registrations=2 closed-fd-token-still-reported shared-OFD last-close-removes");return 0;}
static int service(int file){int ep=epoll_create1(EPOLL_CLOEXEC),p[2][2],sinkfd[2];CHECK(ep>=0 && !pipe2(p[0],O_NONBLOCK|O_CLOEXEC) && !pipe2(p[1],O_NONBLOCK|O_CLOEXEC));
    if(file){char path[]="/tmp/capstone-output-XXXXXX";sinkfd[0]=mkstemp(path);CHECK(sinkfd[0]>=0 && !unlink(path));sinkfd[1]=dup(sinkfd[0]);CHECK(sinkfd[1]>=0);}
    else CHECK(!pipe2(sinkfd,O_NONBLOCK|O_CLOEXEC));
    struct cs_service s;struct cs_linux a;CHECK(cs_init(&s,3,1000)==CS_OK);csl_init(&a,&s,ep);CHECK(!csl_open(&a,0,p[0][0],1) && !csl_open(&a,1,p[1][0],1));
    unsigned char expected[8*42];size_t expected_n=0;
    for(unsigned id=0;id<8;++id){struct cp_message m={(uint16_t)id,id%3==0?32u:3u,{0}};for(size_t j=0;j<m.length;++j)m.payload[j]=(unsigned char)(id+j);unsigned char wire[42];size_t n;CHECK(cp_encode(&m,wire,42,&n)==CP_OK);CHECK(write(p[id%2][1],wire,n)==(ssize_t)n);
        struct epoll_event ev;CHECK(epoll_wait(ep,&ev,1,1000)==1 && csl_event(&a,ev.data.u64)==CS_OK);
        for(unsigned turn=0;turn<20 && a.session[id%2].ready;++turn){struct cs_turn t=csl_turn(&a,id%2,4,id!=3);CHECK(t.status==CS_PENDING || t.status==CS_MORE);}
        CHECK(!a.session[id%2].ready);
        if(id!=3){CHECK(cs_begin(&s)==CS_OK && cs_step(&s,csl_write,&sinkfd[1])==CS_DONE);memcpy(expected+expected_n,wire,n);expected_n+=n;}
    }
    if(file)CHECK(lseek(sinkfd[0],0,SEEK_SET)==0);
    unsigned char got[8*42];CHECK(read(sinkfd[0],got,sizeof got)==(ssize_t)expected_n && !memcmp(got,expected,expected_n));CHECK(s.accepted==7 && s.delivered==7 && s.denied==1 && cs_conserved(&s));
    CHECK(cs_close(&s,0)==CS_CLOSED);for(unsigned i=0;i<2;++i)CHECK(!csl_close(&a,i) && !close(p[i][1]));CHECK(!close(sinkfd[0]) && !close(sinkfd[1]) && !close(ep));
    printf("native-two-input-service accepted=7 denied=1 output-bytes=%zu file=%d no-durability-claim\n",expected_n,file);return 0;}
int main(int argc,char **argv){if(argc!=2)return 2;
    if(!strcmp(argv[1],"fairness"))return fairness();
    if(!strcmp(argv[1],"duplicate"))return duplicate();
    if(!strcmp(argv[1],"service"))return service(0);
    if(!strcmp(argv[1],"file"))return service(1);
    return 2;}
