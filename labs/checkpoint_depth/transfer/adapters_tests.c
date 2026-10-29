#define _POSIX_C_SOURCE 200809L
#include "adapters.h"
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"CPX_CHECK: %s:%d: %s\n",__FILE__,__LINE__,#x);exit(1);}}while(0)
static int unregister_later(void *context,uint64_t cookie){CHECK(cookie==19);++*(unsigned *)context;return 0;}
static void owner(void){
 struct ow_allocator a={0};struct cpx_owner o={0};o.cookie=19;size_t lengths[3]={1,2,3};unsigned requests=0;
 CHECK(ow_construct(&o.owner,ow_allocator(&a),lengths)==OW_OK && a.live==3);
 CHECK(cpx_owner_close(&o,unregister_later,&requests)==OW_BUSY && a.live==3 && requests==1);
 CHECK(cpx_owner_close(&o,unregister_later,&requests)==OW_BUSY && requests==1);
 CHECK(cpx_owner_ack(&o,18,1)==OW_STALE && cpx_owner_ack(&o,19,0)==OW_BUSY && a.live==3);
 CHECK(cpx_owner_ack(&o,19,1)==OW_OK && a.live==0 && a.releases==3);
 CHECK(cpx_owner_ack(&o,19,1)==OW_STALE && ow_destroy(&o.owner)==OW_OK && a.releases==3);
 printf("COST owner_bytes=%zu adapter_bytes=%zu extra_control_bytes=%zu unregister_callbacks=%u releases=%zu\n",sizeof(struct ow_owner),sizeof(struct cpx_owner),sizeof(struct cpx_owner)-sizeof(struct ow_owner),requests,a.releases);
}
struct cpx_script { const long *values;unsigned index; };
static struct cpw_result script(void *context,const unsigned char *data,size_t n){struct cpx_script *s=context;CHECK(data && n);return cpw_raw(s->values[s->index++]);}
static struct cpw_result libc_write(void *context,const unsigned char *data,size_t n){int fd=*(int *)context;ssize_t r=write(fd,data,n);int saved=r<0?errno:0;return cpw_libc((long)r,saved);}
static void progress(void){
 const long values[]={3,-4,2,-5};const unsigned char bytes[]="123456789";struct cpx_script s={values,0};
 struct cpx_progress p={{0,ER_AGAIN},{77,0,0,0},0};struct cpw_transfer baseline;CHECK(cpw_begin(&baseline,bytes,9,77)==CPW_PENDING);struct cpx_script other={values,0};
 for(unsigned i=0;i<4;++i){(void)cpx_progress_step(&p,bytes,9,script,&s);(void)cpw_step(&baseline,script,&other);CHECK(p.progress.next==baseline.accepted && p.error.primary==baseline.error.primary);}
 CHECK(p.progress.code==ER_IO && p.progress.next==5 && p.error.primary==5 && p.callbacks==4);
 cpx_progress_cleanup(&p,9);cpw_cleanup(&baseline,9);CHECK(p.error.primary==5 && p.error.cleanup==9 && baseline.error.cleanup==9);
 CHECK(cpx_progress_step(&p,bytes,9,script,&s)==ER_IO && p.callbacks==4);
 printf("COST progress_product_bytes=%zu core_result_bytes=%zu adapter_state_bytes=%zu core_state_bytes=%zu callback_calls=%u payload_copies=0\n",sizeof(er_progress),sizeof(struct cpw_result),sizeof(struct cpx_progress),sizeof(struct cpw_transfer),p.callbacks);
}
static void actual_libc(void){
 int fds[2];CHECK(pipe(fds)==0);CHECK(fcntl(fds[1],F_SETFL,O_NONBLOCK)==0 && fcntl(fds[0],F_SETFL,O_NONBLOCK)==0);
 unsigned char fill[4096];memset(fill,0x55,sizeof fill);size_t filled=0;
 for(;;){ssize_t n=write(fds[1],fill,sizeof fill);if(n<0){CHECK(errno==EAGAIN);break;}CHECK(n>0);filled+=(size_t)n;CHECK(filled<16777216);}
 const unsigned char bytes[]="ninebytes";struct cpw_transfer t;CHECK(cpw_begin(&t,bytes,9,99)==CPW_PENDING);
 CHECK(cpw_step(&t,libc_write,&fds[1])==CPW_PENDING && t.accepted==0);
 struct pollfd ready={fds[1],POLLOUT,0};CHECK(poll(&ready,1,0)==0);
 size_t drained=0;while(drained<filled){ssize_t n=read(fds[0],fill,sizeof fill);CHECK(n>0);drained+=(size_t)n;}
 CHECK(poll(&ready,1,1000)==1 && (ready.revents&POLLOUT));
 CHECK(cpw_step(&t,libc_write,&fds[1])==CPW_DONE && t.accepted==9);
 unsigned char got[9];CHECK(read(fds[0],got,9)==9 && !memcmp(got,bytes,9));
 CHECK(close(fds[1])==0);CHECK(cpw_begin(&t,bytes,9,100)==CPW_PENDING);
 /* No concurrent descriptor allocation: the deliberately closed integer is not reused. */
 CHECK(cpw_step(&t,libc_write,&fds[1])==CPW_FAILED && t.error.primary==EBADF && !t.accepted);CHECK(close(fds[0])==0);
}
int main(void){owner();progress();actual_libc();puts("CPX_PASS modern ownership/result adapters and actual libc nonblocking pipe");return 0;}
