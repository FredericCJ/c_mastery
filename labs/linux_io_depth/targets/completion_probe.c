#define _GNU_SOURCE
#include "raw_ring.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#define C(x) do{if(!(x)){fprintf(stderr,"B06_IO_CHECK:%d:%s errno=%d\n",__LINE__,#x,errno);exit(1);}}while(0)
static struct cm_raw_ring ring;
static void push(struct io_uring_sqe s){C(cm_ring_push(&ring,&s)==0);}
static void submit(void){C(cm_ring_submit(&ring)==0);}
static struct io_uring_cqe take(void){struct io_uring_cqe c;C(cm_ring_wait(&ring,&c)==0);printf("CQE id=%llu result=%d flags=%u\n",(unsigned long long)c.user_data,c.res,c.flags);return c;}
static void cancel(uint64_t id,uint64_t event){struct io_uring_sqe s={0};s.opcode=IORING_OP_ASYNC_CANCEL;s.addr=id;s.user_data=event;push(s);submit();}
static void read_one(int fd,unsigned char *b,uint64_t id,unsigned flags){struct io_uring_sqe s={0};s.opcode=IORING_OP_READ;s.fd=fd;s.addr=(uintptr_t)b;s.len=1;s.user_data=id;s.flags=(unsigned char)flags;push(s);}
static void pair(uint64_t a,int ar,uint64_t b,int br){int seen=0;for(int i=0;i<2;i++){struct io_uring_cqe c=take();if(c.user_data==a){C(!(seen&1)&&c.res==ar);seen|=1;}else{C(c.user_data==b&&!(seen&2)&&c.res==br);seen|=2;}C(!(c.flags&IORING_CQE_F_MORE));}C(seen==3);}
static void ring_cases(void){
 C(cm_ring_init(&ring,8)==0);int p[2];C(pipe(p)==0);unsigned char b=0xCC;
 read_one(p[0],&b,0x100000003ULL,0);submit();cancel(0x100000003ULL,101);pair(0x100000003ULL,-ECANCELED,101,0);C(b==0xCC);
 cancel(0x100000003ULL,102);struct io_uring_cqe c=take();C(c.user_data==102&&c.res==-ENOENT);
 read_one(p[0],&b,0x200000003ULL,0);submit();cancel(0x100000003ULL,103);c=take();C(c.user_data==103&&c.res==-ENOENT);
 C(write(p[1],"K",1)==1);c=take();C(c.user_data==0x200000003ULL&&c.res==1&&b=='K');
 b=0xCC;read_one(p[0],&b,201,IOSQE_IO_LINK);struct __kernel_timespec ts={0,5000000};struct io_uring_sqe timeout={0};timeout.opcode=IORING_OP_LINK_TIMEOUT;timeout.addr=(uintptr_t)&ts;timeout.len=1;timeout.user_data=202;push(timeout);submit();pair(201,-ECANCELED,202,-ETIME);C(b==0xCC);
 struct io_uring_sqe poll_s={0};poll_s.opcode=IORING_OP_POLL_ADD;poll_s.fd=p[0];poll_s.poll32_events=POLLIN;poll_s.len=IORING_POLL_ADD_MULTI;poll_s.user_data=301;push(poll_s);submit();
 for(int i=0;i<2;i++){C(write(p[1],"M",1)==1);c=take();C(c.user_data==301&&(c.res&POLLIN)&&(c.flags&IORING_CQE_F_MORE));C(read(p[0],&b,1)==1&&b=='M');}
 cancel(301,302);int terminal=0,canceled=0;for(int i=0;i<8&&(!terminal||!canceled);i++){c=take();if(c.user_data==302){C(c.res==0);canceled++;}else{C(c.user_data==301);if(!(c.flags&IORING_CQE_F_MORE)){C(c.res==-ECANCELED);terminal++;}}}C(terminal==1&&canceled==1);
 /* SQ saturation and wrap-around without touching operation payloads. */
 for(unsigned j=0;j<4;j++){unsigned seen=0;C(ring.capacity<=16);for(unsigned i=0;i<ring.capacity;i++){struct io_uring_sqe n={0};n.opcode=IORING_OP_NOP;n.user_data=400+i;push(n);}struct io_uring_sqe n={0};C(cm_ring_push(&ring,&n)==-EAGAIN);submit();for(unsigned i=0;i<ring.capacity;i++){c=take();C(c.res==0&&c.user_data>=400&&c.user_data<400+ring.capacity);unsigned bit=1u<<(c.user_data-400);C(!(seen&bit));seen|=bit;}C(seen==(1u<<ring.capacity)-1);}
 /* A MORE result is not completion of a multishot operation's lifetime.
    Keep the linked timeout alive through an observed event and terminal CQEs. */
 poll_s.user_data=501;poll_s.flags=IOSQE_IO_LINK;ts.tv_nsec=100000000;timeout.user_data=502;push(poll_s);push(timeout);submit();C(write(p[1],"T",1)==1);c=take();C(c.user_data==501&&(c.res&POLLIN)&&(c.flags&IORING_CQE_F_MORE));C(read(p[0],&b,1)==1&&b=='T');pair(501,-ECANCELED,502,-ETIME);
 close(p[0]);close(p[1]);cm_ring_close(&ring);puts("raw-ring terminal ownership PASS");
}
static void transfer_cases(void){int a[2],b[2];C(pipe2(a,O_NONBLOCK)==0&&pipe2(b,O_NONBLOCK)==0);C(write(a[1],"synthetic",9)==9);C(splice(a[0],NULL,b[1],NULL,9,SPLICE_F_NONBLOCK)==9);char data[8192];C(read(a[0],data,1)==-1&&errno==EAGAIN);C(read(b[0],data,9)==9&&!memcmp(data,"synthetic",9));close(a[0]);close(a[1]);close(b[0]);close(b[1]);
 C(pipe2(a,O_NONBLOCK)==0);memset(data,'x',sizeof data);int total=0;ssize_t n;while((n=write(a[1],data,sizeof data))>0)total+=(int)n;C(errno==EAGAIN&&total>=4096);C(read(a[0],data,4096)==4096);char x[3000],y[3000];memset(x,'A',sizeof x);memset(y,'B',sizeof y);struct iovec v[3]={{x,3000},{NULL,0},{y,3000}};n=writev(a[1],v,3);C(n>0&&n<6000);ssize_t progress=n;int discarded=total-4096;while(discarded){ssize_t got=read(a[0],data,(size_t)discarded<sizeof data?(size_t)discarded:sizeof data);C(got>0);discarded-=(int)got;}ssize_t got=read(a[0],data,(size_t)n);C(got==n);for(ssize_t i=0;i<n;i++)C(data[i]==(i<3000?'A':'B'));printf("writev partial=%zd of6000; zero-length vector preserved prefix\n",progress);close(a[0]);close(a[1]);puts("splice/writev PASS");}
int main(void){alarm(20);transfer_cases();ring_cases();return 0;}
