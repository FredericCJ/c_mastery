/* Original one-client QNX8 blocked-close experiment. Requires the actual SDK. */
#ifndef __QNXNTO__
#error QNX_SDP8_TARGET_REQUIRED
#endif
#include <errno.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include <sys/dispatch.h>
#include <sys/neutrino.h>
#include <unistd.h>

static struct {
    rcvid_t rcvid;
    struct _msg_info client;
    iofunc_ocb_t *ocb;
    unsigned attempted;
} held;

static int close_signal(resmgr_context_t *ctp,io_close_t *msg,iofunc_ocb_t *ocb)
{
    if(ctp->rcvid==0){
        /* A dying client cannot leave a pending OCB pointer after default cleanup. */
        if(held.ocb==ocb){held.rcvid=0;held.ocb=NULL;puts("QC_SIGNAL:disconnect-abort");fflush(stdout);}
        return iofunc_close_dup_default(ctp,msg,ocb);
    }
    if(held.rcvid)return EBUSY;
    if(!held.attempted){
        struct _msg_info info;
        if(MsgInfo(ctp->rcvid,&info)==-1 || info.pid<=0 || info.tid<=0)return EIO;
        held.attempted=1;held.rcvid=ctp->rcvid;held.client=info;held.ocb=ocb;
        /* The library has already received this _IO_CLOSE: its caller is REPLY-blocked.
           No default close or reply precedes this exact-thread signal. */
        printf("QC_SIGNAL:held rcvid=%ju pid=%jd tid=%jd coid=%jd\n",
               (uintmax_t)held.rcvid,(intmax_t)info.pid,(intmax_t)info.tid,(intmax_t)info.coid);
        fflush(stdout);
        if(SignalKill(0,info.pid,info.tid,SIGUSR1,SI_USER,0)==-1){
            held.rcvid=0;held.ocb=NULL;puts("QC_SIGNAL:send-failed");fflush(stdout);return EIO;
        }
        puts("QC_SIGNAL:sent-exact-thread");fflush(stdout);
        return _RESMGR_NOREPLY;
    }
    puts("QC_SIGNAL:default-close");fflush(stdout);
    return iofunc_close_dup_default(ctp,msg,ocb);
}

static int unblock_signal(resmgr_context_t *ctp,io_pulse_t *msg,iofunc_ocb_t *ocb)
{
    struct _msg_info info;
    int status=iofunc_unblock_default(ctp,msg,ocb);
    if(status!=_RESMGR_DEFAULT)return status;
    /* A pulse is not authority to reply to a reused receive ID. Check both
       retained identity and the current kernel flag, as the QNX8 guide requires. */
    if(!held.rcvid || ctp->rcvid!=held.rcvid || ocb!=held.ocb ||
       MsgInfo(ctp->rcvid,&info)==-1 || !(info.flags&_NTO_MI_UNBLOCK_REQ) ||
       info.pid!=held.client.pid || info.tid!=held.client.tid ||
       info.coid!=held.client.coid || info.scoid!=held.client.scoid)
        return _RESMGR_NOREPLY;
    printf("QC_SIGNAL:unblock-pending rcvid=%ju pid=%jd tid=%jd coid=%jd\n",
           (uintmax_t)held.rcvid,(intmax_t)info.pid,(intmax_t)info.tid,(intmax_t)info.coid);
    fflush(stdout);
    /* -1 is permitted only here, on an actual still-pending unblock request.
       It selects the kernel's stored EINTR; no numeric EINTR is injected. */
    if(MsgError(held.rcvid,-1)==-1){puts("QC_SIGNAL:reply-failed");fflush(stdout);return _RESMGR_NOREPLY;}
    held.rcvid=0;held.ocb=NULL;puts("QC_SIGNAL:kernel-error-replied");fflush(stdout);
    return _RESMGR_NOREPLY;
}

int main(void)
{
    static resmgr_connect_funcs_t connect;static resmgr_io_funcs_t io;static iofunc_attr_t attr;
    resmgr_attr_t config={0};dispatch_t *dispatch;dispatch_context_t *context;
    char path[64];int n=snprintf(path,sizeof path,"/dev/cmastery-signal-close-%ld",(long)getpid());
    if(n<0||(size_t)n>=sizeof path)return 1;
    dispatch=dispatch_create();if(!dispatch)return 2;
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS,&connect,_RESMGR_IO_NFUNCS,&io);
    io.close_dup=close_signal;io.unblock=unblock_signal;iofunc_attr_init(&attr,S_IFNAM|0600,NULL,NULL);
    config.nparts_max=1;config.msg_max_size=2048;
    if(resmgr_attach(dispatch,&config,path,_FTYPE_ANY,0,&connect,&io,&attr)<0){perror("resmgr_attach");return 3;}
    context=dispatch_context_alloc(dispatch);if(!context)return 4;
    printf("QC_SIGNAL_READY:%s\n",path);fflush(stdout);
    /* One dispatch thread; one same-UID, dedicated original probe; no file locks. */
    for(;;){context=dispatch_block(context);if(!context)return 5;dispatch_handler(context);}
}
