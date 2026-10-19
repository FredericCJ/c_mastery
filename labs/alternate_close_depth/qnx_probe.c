/* Target-only: compile with an authorized QNX SDP8 sysroot, never host shims. */
#ifndef __QNXNTO__
#error QNX_SDP8_TARGET_REQUIRED
#endif
#include "close_owner.h"
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <unistd.h>
static QCRaw actual_close(void *p,int fd)
{
    unsigned *calls=p;(*calls)++;int rc=close(fd);return (QCRaw){rc,rc==-1?errno:0};
}
int main(int argc,char **argv)
{
    const char prefix[]="/dev/cmastery-close-";struct utsname u;struct stat before,after;QCOwner owner;QCObservation first,last;
    unsigned calls=0;int old,fd,closed_errno,closed_rc;
    if(argc!=2||strncmp(argv[1],prefix,sizeof prefix-1)||!argv[1][sizeof prefix-1])return 2;
    for(const char *p=argv[1]+sizeof prefix-1;*p;p++)if(*p<'0'||*p>'9')return 2;
    if(uname(&u)||strcmp(u.sysname,"QNX")||strncmp(u.release,"8.",2))return 3;
    if(pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&old))return 4;
    /* One client, one fresh manager. No duplicate or concurrently borrowed fd. */
    fd=open(argv[1],O_RDWR);if(fd<0||fstat(fd,&before)||qc_init(&owner,fd)!=QC_OK)return 5;
    enum QCStatus a=qc_once(&owner,actual_close,&calls,&first);
    int live=fstat(fd,&after)==0;
    int same=live&&before.st_dev==after.st_dev&&before.st_ino==after.st_ino;
    printf("QC_QNX_FIRST rc=%d error=%d eintr=%d retained=%d calls=%u live=%d same=%d\n",a,first.error,first.error==EINTR,owner.fd==fd&&owner.state==QC_RETRY,calls,live,same);
    if(a!=QC_AGAIN||first.error!=EINTR||calls!=1||!same||owner.fd!=fd||owner.state!=QC_RETRY)return 6;
    enum QCStatus b=qc_once(&owner,actual_close,&calls,&last);
    errno=0;closed_rc=fstat(fd,&after);closed_errno=errno;
    printf("QC_QNX_FINAL rc=%d state=%d calls=%u ebadf=%d\n",b,owner.state,calls,closed_rc==-1&&closed_errno==EBADF);
    if(b!=QC_OK||owner.state!=QC_CLOSED||calls!=2||closed_rc!=-1||closed_errno!=EBADF)return 7;
    if(pthread_setcancelstate(old,NULL))return 8;
    printf("QC_QNX_ID sysname=%s release=%s machine=%s\n",u.sysname,u.release,u.machine);
    puts("QC_QNX_OK:resource-manager-error-path");return 0;
}
