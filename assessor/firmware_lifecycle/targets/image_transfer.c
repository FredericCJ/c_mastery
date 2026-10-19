#include "image_transfer.h"
#include <string.h>
#ifndef FI_MUTANT
#define FI_MUTANT 0
#endif
static uint32_t crc(const unsigned char*p){uint32_t x=UINT32_MAX;for(unsigned i=0;i<32;i++){x^=p[i];for(unsigned b=0;b<8;b++)x=x&1?(x>>1)^UINT32_C(0xedb88320):x>>1;}return ~x;}
int fi_init(FIImage*s,unsigned unit){if(unit!=4&&unit!=8)return 0;memset(s->bytes,255,32);s->unit=unit;s->completed=0;return 1;}
int fi_program(FIImage*s,const unsigned char*image,unsigned index,unsigned bits,int complete){if((s->unit!=4&&s->unit!=8)||index>=32/s->unit||bits>s->unit*8||(complete!=0&&complete!=1)||(complete&&bits!=s->unit*8))return 0;unsigned offset=index*s->unit;for(unsigned b=0;b<s->unit;b++)if((s->bytes[offset+b]&image[offset+b])!=image[offset+b])return 0;s->completed&=~(1u<<index);for(unsigned bit=0;bit<bits;bit++)s->bytes[offset+bit/8]&=(unsigned char)(image[offset+bit/8]|(unsigned char)~(1u<<(bit%8)));if(complete)s->completed|=1u<<index;return 1;}
int fi_commit(const FIImage*s,const unsigned char*image,uint32_t seq,unsigned char record[24]){if((s->unit!=4&&s->unit!=8)||!seq)return 0;if(FI_MUTANT!=1&&s->completed!=(1u<<(32/s->unit))-1)return 0;if(memcmp(s->bytes,image,32))return 0;return lce_record(record,seq,crc(s->bytes));}
int fi_record_matches(const unsigned char image[32],const unsigned char record[24]){uint32_t seq,value;return lce_valid(record,&seq,&value)&&value==crc(image);}
int fi_advance(FIRotation*s){if(!s->limit||s->sequence>=s->limit)return 0;++s->sequence;return 1;}
int fi_rotate(FIRotation*s,uint32_t epoch,unsigned key,uint32_t floor,unsigned verified,int verdict,FIPersist persist,void*ctx){if(s->epoch==UINT32_MAX||epoch!=s->epoch+1||!s->limit||s->sequence!=s->limit||key<1||key>2||key==s->key||(s->revoked&(1u<<key))||floor<s->floor||verified!=key||verdict!=1||!persist)return 0;FIRotation candidate=*s;candidate.epoch=epoch;candidate.sequence=0;candidate.key=key;candidate.floor=floor;if(FI_MUTANT!=2)candidate.revoked|=1u<<s->key;if(FI_MUTANT==3)*s=candidate;if(persist(ctx,&candidate)!=1)return 0;*s=candidate;return 1;}
