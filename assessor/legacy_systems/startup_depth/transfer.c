#include "startup.h"
struct sd_result sd_linux_result(uint64_t raw){struct sd_result out;
 if(raw>=UINT64_MAX-4094){out.failed=1;out.value=0;out.error=(unsigned)(0u-raw);}
 else{out.failed=0;out.value=raw;out.error=0;}return out;
}
enum sd_status sd_transfer_init(struct sd_transfer *x,unsigned char *buffer,size_t length,int reading){
 if(!x||(!buffer&&length)||(reading!=0&&reading!=1))return SD_ARGUMENT;
 x->buffer=buffer;x->length=length;x->offset=0;x->reading=reading;x->status=length?SD_READY:SD_DONE;x->error=0;return SD_OK;
}
enum sd_transfer_status sd_transfer_step(struct sd_transfer *x,sd_backend backend,void *context,size_t budget,int cancel){
 if(!x||!backend||x->offset>x->length||(!x->buffer&&x->length))return SD_BACKEND_BROKEN;
 if(x->status!=SD_READY&&x->status!=SD_BLOCKED&&x->status!=SD_YIELD)return x->status;
 if(cancel){x->status=SD_CANCELLED;return x->status;}
 x->error=0;
 for(size_t attempt=0;attempt<budget;attempt++){
  size_t remaining=x->length-x->offset;
  if(!remaining){x->status=SD_DONE;return x->status;}
  struct sd_result result=backend(context,x->buffer+x->offset,remaining);
  if((result.failed!=0&&result.failed!=1)||(result.failed&&(!result.error||result.error>4095||result.value))||(!result.failed&&result.error)){x->status=SD_BACKEND_BROKEN;return x->status;}
  if(result.failed){
   if(result.error==4)continue;
   x->error=result.error;x->status=result.error==11?SD_BLOCKED:SD_IO_ERROR;return x->status;
  }
  if(result.value>remaining){x->status=SD_BACKEND_BROKEN;return x->status;}
  if(!result.value){x->status=x->reading?SD_EOF:SD_STALLED;return x->status;}
#ifndef SD_MUTANT_RESEND
  x->offset+=(size_t)result.value;
#endif
  if(x->offset==x->length){x->status=SD_DONE;return x->status;}
 }
 x->status=SD_YIELD;return x->status;
}
