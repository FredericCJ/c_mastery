#include "http_depth.h"
int hd_length(const unsigned char *s,size_t n,uint32_t cap,uint32_t *out) {
  if(!s||!n||!out||cap>1024)return 400;
  uint32_t v=0;
  for(size_t i=0;i<n;i++) {
    if(s[i]<'0'||s[i]>'9')return 400;
    unsigned d=s[i]-'0';
#ifndef HD_MUTATE_LENGTH
    if(v>(UINT32_MAX-d)/10u)return 400;
#endif
    v=v*10u+d;
#ifdef HD_MUTATE_LENGTH2
    if(v>cap)return 413;
#endif
  }
  if(v>cap)return 413;
  *out=v;return HD_OK;
}
static unsigned leap(unsigned y) {
#ifdef HD_MUTATE_DATE
  return y%4==0;
#else
  return y%4==0&&(y%100!=0||y%400==0);
#endif
}
static void two(char *p,unsigned n) {p[0]=(char)('0'+n/10);p[1]=(char)('0'+n%10);}
int hd_date(unsigned available,uint32_t days,uint32_t seconds,char *out,size_t cap,size_t *length) {
  if(!length||available>1)return HD_INVALID;
#ifndef HD_MUTATE_DATE2
  if(!available){*length=0;return HD_OK;}
#endif
  if(!out||cap<30||days>2932896u||seconds>86399u)return HD_INVALID;
  unsigned y=1970,m=0,day=days;
  while(day>=365u+leap(y)){day-=365u+leap(y);y++;}
  static const unsigned lengths[12]={31,28,31,30,31,30,31,31,30,31,30,31};
  while(day>=lengths[m]+(m==1?leap(y):0)){day-=lengths[m]+(m==1?leap(y):0);m++;}
  static const char weekdays[]="SunMonTueWedThuFriSat",months[]="JanFebMarAprMayJunJulAugSepOctNovDec";
  char value[30]="Thu, 01 Jan 1970 00:00:00 GMT";
  unsigned w=(days+4u)%7u;
  for(unsigned i=0;i<3;i++){value[i]=weekdays[w*3+i];value[8+i]=months[m*3+i];}
  two(value+5,day+1);two(value+12,y/100);two(value+14,y%100);
  two(value+17,seconds/3600);two(value+20,(seconds/60)%60);two(value+23,seconds%60);
  for(unsigned i=0;i<30;i++)out[i]=value[i];
  *length=29;return HD_OK;
}
static int valid(const struct hd_pool *p,struct hd_handle h) {
  if(!p||h.index>=4)return 0;
  const struct hd_slot *s=&p->slots[h.index];
#ifdef HD_MUTATE_LIFECYCLE
  return s->live!=0;
#else
  return s->live&&s->generation==h.generation;
#endif
}
int hd_admit(struct hd_pool *p,int fd,unsigned flags,struct hd_handle *out) {
  if(!p||!out||fd<0||flags!=1)return HD_INVALID;
  for(unsigned i=0;i<4;i++)if(p->slots[i].live&&p->slots[i].fd==fd)return HD_INVALID;
  for(unsigned i=0;i<4;i++) {
    struct hd_slot *s=&p->slots[i];if(s->live||s->retired||s->generation==UINT32_MAX)continue;
    s->generation++;s->live=1;s->fd=fd;s->interest=1;
    out->index=i;out->generation=s->generation;return HD_OK;
  }
  return HD_FULL;
}
int hd_interest(struct hd_pool *p,struct hd_handle h,unsigned mask) {
  if(!p||mask>3)return HD_INVALID;
  if(!valid(p,h))return HD_STALE;
  p->slots[h.index].interest=mask;return HD_OK;
}
int hd_event(const struct hd_pool *p,struct hd_handle h,int fd,unsigned ready,unsigned *out) {
  if(!p||!out||ready>15||fd<0)return HD_INVALID;
  if(!valid(p,h)||p->slots[h.index].fd!=fd)return HD_STALE;
#ifdef HD_MUTATE_LIFECYCLE2
  *out=ready;
#else
  *out=ready&(p->slots[h.index].interest|12u);
#endif
  return HD_OK;
}
int hd_close(struct hd_pool *p,struct hd_handle h,int *out) {
  if(!p||!out)return HD_INVALID;
  if(!valid(p,h))return HD_STALE;
  struct hd_slot *s=&p->slots[h.index];*out=s->fd;s->live=0;s->interest=0;
  if(s->generation==UINT32_MAX)s->retired=1;
  return HD_OK;
}
