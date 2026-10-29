#include "packet_ring.h"
#include "foundation.h"
#include "hosted.h"
#ifndef ADP_MUTATION
#define ADP_MUTATION 0
#endif
static enum adp_status error(enum ad_packet_error e) {
 switch(e){case AD_PACKET_OK:return ADP_OK;case AD_PACKET_HEADER:return ADP_HEADER;
 case AD_PACKET_KIND:return ADP_KIND;case AD_PACKET_LENGTH:return ADP_LENGTH;
 case AD_PACKET_ACK:return ADP_ACK;}return ADP_INVALID;
}
enum adp_status adp_parse(const unsigned char *p,size_t n,struct ad_packet *out){
 if(!out)return ADP_INVALID;
 struct ad_packet result;enum adp_status s=error(ad_packet(p,n,&result));if(s!=ADP_OK)return s;
 struct f_cursor c={p,n,0};const uint8_t *head,*payload;
 if(f_take(&c,3,&head)!=F_OK||f_take(&c,result.length,&payload)!=F_OK||c.position!=n)return ADP_INVALID;
 result.kind=head[0];result.payload=payload;*out=result;return ADP_OK;
}
enum adp_status adp_build(unsigned kind,const unsigned char *p,size_t n,unsigned char *out,size_t cap,size_t *used){
 size_t total;
 if(!out||!used||(n&&!p))return ADP_INVALID;
 if(kind!=1&&kind!=2)return ADP_KIND;
 if(n>UINT16_MAX||f_checked_size(n,1,3,&total)!=F_OK)return ADP_LENGTH;
 if(kind==2&&n)return ADP_ACK;
 if(total>cap)return ADP_FULL;
 out[0]=(unsigned char)kind;out[1]=(unsigned char)(n>>8);out[2]=(unsigned char)n;
 for(size_t i=0;i<n;i++)out[3+i]=p[i];
 *used=total;return ADP_OK;
}
enum adp_status adp_init(struct adp_ring *r,size_t origin,uint32_t generation){
 if(!r||origin>=ADP_CAP||!generation)return ADP_INVALID;
 struct adp_ring fresh={0};fresh.head=origin;fresh.next=generation;*r=fresh;return ADP_OK;
}
enum adp_status adp_feed(struct adp_ring *r,const unsigned char *p,size_t n){
 if(!r||(n&&!p))return ADP_INVALID;
 if(r->closed)return ADP_CANCELLED;
 if(n>ADP_CAP-r->count)return ADP_FULL;
 for(size_t i=0;i<n;i++)r->bytes[(r->head+r->count+i)%ADP_CAP]=p[i];
 r->count+=n;return ADP_OK;
}
enum adp_status adp_discard(struct adp_ring *r,size_t n){
 if(!r)return ADP_INVALID;
 if(r->active)return ADP_BUSY;
 if(n>r->count)return ADP_PENDING;
 r->head=(r->head+n)%ADP_CAP;r->count-=n;return ADP_OK;
}
enum adp_status adp_validate(const struct adp_ring *r,struct adp_token t){
 if(!r)return ADP_INVALID;
 if(t.owner!=r||!r->active||(ADP_MUTATION!=3&&t.generation!=r->generation))return ADP_STALE;
 return r->cancelled?ADP_CANCELLED:ADP_OK;
}
enum adp_status adp_acquire(struct adp_ring *r,size_t n,struct adp_lease *out){
 if(!r||!out)return ADP_INVALID;
 if(r->closed)return ADP_CANCELLED;
 if(r->active)return ADP_BUSY;
 if(r->retired&&ADP_MUTATION!=4)return ADP_RETIRED;
 if(n>r->count)return ADP_PENDING;
 if(n<3)return ADP_HEADER;
 unsigned kind=r->bytes[r->head];
 size_t length=((size_t)r->bytes[(r->head+1)%ADP_CAP]<<8)|r->bytes[(r->head+2)%ADP_CAP];
 if(kind!=1&&kind!=2)return ADP_KIND;
 if(length!=n-3)return ADP_LENGTH;
 if(kind==2&&length)return ADP_ACK;
 struct adp_lease result={0};result.kind=kind;result.length=length;
 if(length){size_t start=(r->head+3)%ADP_CAP;size_t first=ADP_CAP-start;if(first>length)first=length;
  result.part[0]=(struct adp_part){r->bytes+start,first};
  if(length>first)result.part[1]=(struct adp_part){r->bytes,ADP_MUTATION==1?0:length-first};
 }
 r->generation=r->next;r->active=1;r->cancelled=0;r->held=n;
 if(r->next==UINT32_MAX){r->retired=1;if(ADP_MUTATION==4)r->next=1;}else r->next++;
 result.token=(struct adp_token){r,r->generation};*out=result;return ADP_OK;
}
enum adp_status adp_release(struct adp_ring *r,struct adp_token t){
 enum adp_status s=adp_validate(r,t);if(s!=ADP_OK&&s!=ADP_CANCELLED)return s;
 r->head=(r->head+r->held)%ADP_CAP;r->count-=r->held;r->held=0;r->active=0;r->cancelled=0;return ADP_OK;
}
enum adp_status adp_cancel(struct adp_ring *r,struct adp_token t){
 enum adp_status s=adp_validate(r,t);if(s!=ADP_OK&&s!=ADP_CANCELLED)return s;
 r->cancelled=1;if(ADP_MUTATION==2)return adp_release(r,t);return ADP_OK;
}
enum adp_status adp_close(struct adp_ring *r){
 if(!r)return ADP_INVALID;
 r->closed=1;if(r->active)r->cancelled=1;return ADP_OK;
}
enum adp_status adp_copy(const struct adp_ring *r,struct adp_lease v,unsigned char *out,size_t cap){
 if(!out&&v.length)return ADP_INVALID;
 enum adp_status s=adp_validate(r,v.token);if(s!=ADP_OK&&s!=ADP_CANCELLED)return s;
 if(v.length>cap)return ADP_FULL;
 size_t at=0;for(unsigned k=0;k<2;k++)for(size_t i=0;i<v.part[k].length;i++)out[at++]=v.part[k].data[i];
 return ADP_OK;
}
enum adp_status adp_record(const struct adp_ring *r,struct adp_lease v,unsigned version,struct ar_record *out){
 if(!out)return ADP_INVALID;
 if(v.kind!=1||v.length>5)return ADP_CODEC;
 unsigned char bytes[5];enum adp_status s=adp_copy(r,v,bytes,sizeof bytes);if(s!=ADP_OK)return s;
 struct ar_record result;if(ar_decode(ADP_MUTATION==6?1:version,bytes,v.length,&result)!=AR_OK)return ADP_CODEC;
 *out=result;return ADP_OK;
}
enum adp_status adp_read_pipe(int fd,unsigned char *storage,size_t cap,struct ad_packet *out){
 if(!out||(!storage&&cap))return ADP_INVALID;
 size_t n=0;int r=hs_read_frame(fd,storage,cap,&n);
 if(r){if(ADP_MUTATION==5)out->kind=0;return r==-2?ADP_EOF:r==-3?ADP_OVERSIZE:ADP_IO;}
 return adp_parse(storage,n,out);
}
