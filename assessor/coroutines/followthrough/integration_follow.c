#include "integration_follow.h"
#include "hosted.h"
#ifndef CI_MUTANT
#define CI_MUTANT 0
#endif
int ci_parse_cursor(struct f_cursor*c,const struct co_parser*p,size_t pn,unsigned budget,size_t*consumed,unsigned*steps){
 if(!c||!consumed||!steps||c->position>c->length||(!c->data&&c->length))return CO_INVALID;
 struct f_cursor next=*c;size_t end=0;const unsigned char*start=next.length-next.position?next.data+next.position:NULL;
 int r=co_parse(p,pn,start,next.length-next.position,0,budget,&end,steps);
 if(r){if(CI_MUTANT==1&&c->position<c->length)c->position++;return r;}
 const uint8_t*ignored;if(f_take(&next,end,&ignored)!=F_OK)return CO_INVALID;*c=next;*consumed=end;return CO_OK;
}
struct read_port{int data,cancel;long long deadline;struct ci_progress*progress;};
static struct co_read_result read_port(void*v,unsigned char*out,size_t n){struct read_port*p=v;p->progress->reads++;ssize_t r=hs_read_until(p->data,out,n,p->cancel,p->deadline);p->progress->port_result=(int)r;if(r<0)return(struct co_read_result){CO_READ_ERROR,0};return(struct co_read_result){r?CO_READ_DATA:CO_READ_EOF,(size_t)r};}
static int arm_port(void*v,uint32_t generation){(void)generation;struct read_port*p=v;p->progress->arms++;return 0;}
static void disarm_port(void*v,uint32_t generation){(void)generation;struct read_port*p=v;p->progress->disarms++;}
int ci_receive(const int gates[2],int data,int cancel,long long deadline,struct sf_stream*stream,struct sf_handle*handle,int32_t*value,struct ci_progress*progress){
 if(!gates||data<0||cancel<0||!stream||!handle||!value||!progress)return CO_INVALID;
 *progress=(struct ci_progress){0};unsigned count=0;if(hs_reactor_sum(gates,2,&count))return CO_IO;if(!count||count>8)return CO_RANGE;
 struct co_async async;int r=co_async_init(&async,count,1);if(r)return r;struct read_port port={data,cancel,deadline,progress};struct co_async_port p={&port,read_port,arm_port,disarm_port};unsigned char bytes[8];size_t n=0;
 do{r=co_async_poll(&async,p,bytes,&n);}while(r==CO_PENDING);progress->accepted=async.length;if(r!=CO_DONE)return r;
 struct f_cursor cursor={bytes,n,0};const uint8_t*digits;if(f_take(&cursor,n,&digits)!=F_OK)return CO_INVALID;int32_t number;
 if(f_parse_i32(digits,n,&number)!=F_OK)return CO_NO_MATCH;
 struct sf_stream staged;sf_init(&staged);struct sf_handle staged_handle;if(sf_start(&staged,bytes,n,&staged_handle)!=SF_OK)return CO_RANGE;
 *stream=staged;*handle=(struct sf_handle){stream,stream->generation};*value=CI_MUTANT==2?0:number;return CO_DONE;
}
