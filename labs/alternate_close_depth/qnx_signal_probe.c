/* Real QNX8 libc close, blocked-request SIGUSR1 and owner-retention probe. */
#ifndef __QNXNTO__
#error QNX_SDP8_TARGET_REQUIRED
#endif
#include "close_owner.h"
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
static volatile sig_atomic_t caught;
static void handler(int signo){if(signo==SIGUSR1)caught=1;}
static struct qc_raw actual_close(void *p,int fd)
{
    unsigned *calls=p;(*calls)++;int rc=close(fd);return (struct qc_raw){rc,rc==-1?errno:0};
}
int main(int argc,char **argv)
{
    const char prefix[]="/dev/cmastery-signal-close-";
    struct utsname u;struct stat before,after;struct sigaction action={0};sigset_t set;
    struct qc_owner owner;struct qc_observation first,last;unsigned calls=0;int old,fd,closed_rc,closed_errno;
    if(argc!=2||strncmp(argv[1],prefix,sizeof prefix-1)||!argv[1][sizeof prefix-1])return 2;
    for(const char *p=argv[1]+sizeof prefix-1;*p;p++)if(*p<'0'||*p>'9')return 2;
    if(uname(&u)||strcmp(u.sysname,"QNX")||strncmp(u.release,"8.",2))return 3;
    if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&old))return 4;
    action.sa_handler=handler;action.sa_flags=0;
    if(sigemptyset(&action.sa_mask)||sigaction(SIGUSR1,&action,NULL)||
       sigemptyset(&set)||sigaddset(&set,SIGUSR1)||pthread_sigmask(SIG_UNBLOCK,&set,NULL))return 5;
    fd=open(argv[1],O_RDWR);if(fd<0||fstat(fd,&before)||qc_init(&owner,fd)!=QC_OK)return 6;
    printf("QC_SIGNAL_PROBE:handler-installed-before-close pid=%jd\n",(intmax_t)getpid());fflush(stdout);
    enum qc_status a=qc_once(&owner,actual_close,&calls,&first);
    int live=fstat(fd,&after)==0;
    int same=live&&before.st_dev==after.st_dev&&before.st_ino==after.st_ino;
    printf("QC_SIGNAL_FIRST rc=%d eintr=%d retained=%d calls=%u live=%d same=%d caught=%d\n",
           a,first.error==EINTR,owner.fd==fd&&owner.state==QC_RETRY,calls,live,same,(int)caught);
    if(a!=QC_AGAIN||first.error!=EINTR||calls!=1||!same||owner.fd!=fd||owner.state!=QC_RETRY||caught!=1)return 7;
    enum qc_status b=qc_once(&owner,actual_close,&calls,&last);
    errno=0;closed_rc=fstat(fd,&after);closed_errno=errno;
    printf("QC_SIGNAL_FINAL rc=%d state=%d calls=%u ebadf=%d\n",b,owner.state,calls,closed_rc==-1&&closed_errno==EBADF);
    if(b!=QC_OK||owner.state!=QC_CLOSED||calls!=2||closed_rc!=-1||closed_errno!=EBADF)return 8;
    if(pthread_setcancelstate(old,NULL))return 9;
    printf("QC_QNX_ID sysname=%s release=%s machine=%s\n",u.sysname,u.release,u.machine);
    puts("QC_QNX_OK:blocked-close-signal");return 0;
}
