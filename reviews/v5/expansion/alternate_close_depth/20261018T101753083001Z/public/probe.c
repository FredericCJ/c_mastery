#include "close_owner.h"
#include <stdio.h>
typedef struct {QCRaw raw;unsigned calls;} P;
static QCRaw call(void *p,int fd){P *v=p;(void)fd;v->calls++;return v->raw;}
int main(void)
{
    int rc,error;while(scanf("%d %d",&rc,&error)==2){QCOwner o;QCObservation v;P p={{rc,error},0};int fd=99;
      if(qc_init(&o,7)!=QC_OK)return 1;
      enum QCStatus first=qc_once(&o,call,&p,&v);enum QCStatus second=qc_handoff(&o,&fd);
      printf("%d %d %d %u %d %d\n",first,v.state,v.error,p.calls,second,fd);
    }return 0;
}
