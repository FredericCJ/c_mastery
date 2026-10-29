#include "flow.h"
#include <limits.h>
#include <string.h>
static struct hf_slot *slot(struct hf_pool *p, struct hf_handle h) {
  return h.index<4 && p->slot[h.index].generation==h.generation ? &p->slot[h.index] : 0;
}
int hf_admit(struct hf_pool *p,int fd,unsigned flags,struct hf_handle *out) {
  if(fd<0 || flags!=3) return -1;
  for(unsigned i=0;i<4;i++) if(p->slot[i].live && p->slot[i].fd==fd) return -1;
  for(unsigned i=0;i<4;i++) {
    struct hf_slot *s=&p->slot[i];
    if(s->live || s->pending || s->retired) continue;
    if(s->generation==UINT32_MAX) {s->retired=1;continue;}
    s->generation++;s->fd=fd;s->live=1;*out=(struct hf_handle){i,s->generation};return 0;
  }
  return 1;
}
int hf_pin(struct hf_pool *p,struct hf_handle h) {
  struct hf_slot *s=slot(p,h);if(!s || !s->live || s->pending==UINT_MAX) return -1;
  s->pending++;return 0;
}
int hf_callback(struct hf_pool *p,struct hf_handle h,int *out) {
  struct hf_slot *s=slot(p,h);if(!s || !s->pending) return -1;
  s->pending--;if(!s->live) return 1;*out=s->fd;return 0;
}
int hf_close(struct hf_pool *p,struct hf_handle h,int *out) {
  struct hf_slot *s=slot(p,h);if(!s || !s->live) return -1;
  *out=s->fd;s->fd=-1;s->live=0;return 0;
}
int hf_accept_batch(struct hf_pool *p,struct hf_accept ops,struct hf_handle handles[4],unsigned *count) {
  *count=0;
  for(unsigned attempt=0;attempt<4;attempt++) {
    int fd=ops.accept(ops.context);if(fd==-1)continue;if(fd==-2)return 0;if(fd<0)return -1;
    struct hf_handle h;
    if(ops.flags(ops.context,fd) || hf_admit(p,fd,3,&h)){ops.close(ops.context,fd);continue;}
    handles[(*count)++]=h;
  }
  return 0;
}
static const char *reason(int n) {
  switch(n) {case 200:return "OK";case 400:return "Bad Request";case 404:return "Not Found";
  case 405:return "Method Not Allowed";case 408:return "Request Timeout";case 413:return "Content Too Large";
  case 414:return "URI Too Long";case 417:return "Expectation Failed";case 431:return "Request Header Fields Too Large";
  default:return "Not Implemented";}
}
static int add(unsigned char *p,size_t cap,size_t *n,const void *s,size_t z) {
  if(z>cap-*n)return -1;
  if(z)memcpy(p+*n,s,z);
  *n+=z;return 0;
}
static size_t number(size_t x,char *s) {
  char reverse[32];size_t n=0;
  do {reverse[n++]=(char)('0'+x%10);x/=10;}while(x);
  for(size_t i=0;i<n;i++)s[i]=reverse[n-1-i];
  return n;
}
int hf_response(const struct http_request *r,const char *date,int close,const void *error,size_t ez,void *v,size_t cap,size_t *length) {
  unsigned char scratch[1600];size_t n=0,z;char num[32],error_text[40];const void *body;const char *why=reason(r->status);int newline=0;
  if(date) {if(strlen(date)!=29)return -1;for(size_t i=0;i<29;i++)if((unsigned char)date[i]<32 || (unsigned char)date[i]>=127)return -1;}
  if(r->status==200) {body=!strcmp(r->target,"/health")?(const void *)"ok\n":r->body;z=!strcmp(r->target,"/health")?3:r->body_length;}
  else if(error) {body=error;z=ez;}
  else {z=strlen(why)+1;if(z>sizeof error_text)return -1;for(size_t i=0;i<z-1;i++)error_text[i]=why[i]>='A' && why[i]<='Z'?(char)(why[i]+32):why[i];body=error_text;newline=1;}
#define BYTES(p,k) do {if(add(scratch,sizeof scratch,&n,p,k))return -1;}while(0)
#define TEXT(s) BYTES(s,strlen(s))
  TEXT("HTTP/1.1 ");size_t k=number((size_t)r->status,num);BYTES(num,k);TEXT(" ");TEXT(why);TEXT("\r\n");
  if(date){TEXT("Date: ");TEXT(date);TEXT("\r\n");}
  TEXT(r->status==200 && !strcmp(r->target,"/echo")?"Content-Type: application/octet-stream\r\n":"Content-Type: text/plain; charset=utf-8\r\n");
  TEXT("Content-Length: ");k=number(z,num);BYTES(num,k);TEXT("\r\n");
  if(r->status==405)TEXT(!strcmp(r->target,"/health")?"Allow: GET, HEAD\r\n":"Allow: POST\r\n");
  TEXT(close||r->close?"Connection: close\r\n\r\n":"Connection: keep-alive\r\n\r\n");
  if(!r->head){BYTES(body,z-(size_t)newline);if(newline)TEXT("\n");}
  if(n>cap)return -1;
  memcpy(v,scratch,n);*length=n;return 0;
#undef BYTES
#undef TEXT
}
static int enqueue(struct hf_flow *f,int interim,uint64_t now,const char *date) {
  if(f->count==f->capacity)return -1;
  struct hf_output *o=&f->output[(f->head+f->count)%f->capacity];o->offset=0;o->started=now;o->interim=(unsigned)interim;o->request=f->requests+1;
  if(interim){const char s[]="HTTP/1.1 100 Continue\r\n\r\n";memcpy(o->bytes,s,sizeof s-1);o->length=sizeof s-1;}
  else {f->requests++;if(f->requests>=4)f->parser.request.close=1;f->closing=(unsigned)f->parser.request.close;
    if(hf_response(&f->parser.request,date,(int)f->closing,0,0,o->bytes,sizeof o->bytes,&o->length))return -1;}
  f->count++;return 0;
}
int hf_init(struct hf_flow *f,unsigned capacity,const char *authority,uint64_t now) {
  if(capacity<1 || capacity>2)return -1;
  memset(f,0,sizeof *f);f->capacity=capacity;f->started=f->progress=now;http_init(&f->parser,authority);return f->parser.state==HTTP_FAILED?-1:0;
}
int hf_feed(struct hf_flow *f,const void *v,size_t n,size_t *used,uint64_t now,const char *authority,const char *date) {
  *used=0;if(f->aborted || now<f->started || now<f->progress)return -1;
  while(*used<n && f->count<f->capacity && !f->closing) {
    size_t take;int r=http_feed(&f->parser,(const unsigned char *)v+*used,n-*used,&take);*used+=take;if(take)f->progress=now;
    if(r==HTTP_CONTINUE){if(enqueue(f,1,now,date))return -1;}
    else if(r==HTTP_DONE || r==HTTP_ERROR) {if(enqueue(f,0,now,date))return -1;if(!f->closing){http_init(&f->parser,authority);f->started=now;}}
    else if(!take)break;
  }
  return 0;
}
int hf_eof(struct hf_flow *f,uint64_t now,const char *date) {
  if(f->closing)return 0;
  if(f->parser.header_bytes || f->parser.request.body_length) {http_eof(&f->parser);if(enqueue(f,0,now,date))return -1;}
  f->closing=1;return 0;
}
int hf_read_event(struct hf_flow *f,int result,uint64_t now,const char *date) {
  if(result==-1 || result==-2 || result>0)return 0;
  if(result==0)return hf_eof(f,now,date);
  f->aborted=1;return -1;
}
int hf_write(struct hf_flow *f,http_write_fn write,void *ctx,uint64_t now) {
  if(!f->count || f->aborted)return 0;
  struct hf_output *o=&f->output[f->head];
  if(now<o->started || now-o->started>=15000){f->aborted=1;return -1;}
  int r=http_drain(write,ctx,o->bytes,o->length,&o->offset);
  if(r<0)f->aborted=1;
  if(r==1){f->head=(f->head+1)%f->capacity;f->count--;}
  return r;
}
int hf_tick(struct hf_flow *f,uint64_t now,const char *date) {
  if(f->aborted)return 3;
  if(now<f->started || now<f->progress){f->aborted=1;return 3;}
  if(f->count) {struct hf_output *o=&f->output[f->head];if(now<o->started || now-o->started>=15000){f->aborted=1;return 3;}}
  if(f->draining)return now<f->drain_started || now-f->drain_started>=1000?2:0;
  if(f->closing && !f->count){f->draining=1;f->drain_started=now;return 1;}
  if(!f->closing && f->count<f->capacity && http_expired(now,f->started,f->progress,15000,5000)){
    f->parser.request.status=408;f->parser.request.close=1;if(enqueue(f,0,now,date)){f->aborted=1;return 3;}}
  return 0;
}
