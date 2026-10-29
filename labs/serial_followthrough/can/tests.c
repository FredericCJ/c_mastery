#include "can.h"
void cn_note(const char *);
void cn_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){cn_note("CN_ASSERT: " #x);return 41;}}while(0)
static int arbitration(void){
 struct sd_can data={0x123,0,0,1,{0xa5}},remote=data,extended=data;int winner=99;
 remote.remote=1;extended.extended=1;extended.identifier=0x123u<<18;
 CHECK(sd_can_arbitration(&data,&remote,&winner)==SD_OK&&winner==-1);
 CHECK(sd_can_arbitration(&remote,&extended,&winner)==SD_OK&&winner==-1);
 unsigned counterexamples=0;
 for(unsigned base=1;base<2048;++base){
  data.identifier=base;extended.identifier=((base-1)<<18)|0x3ffffu;
  CHECK(extended.identifier>data.identifier);
  CHECK(sd_can_arbitration(&extended,&data,&winner)==SD_OK&&winner==-1);++counterexamples;
 }
 remote=data;remote.data[0]^=0xff;
 CHECK(sd_can_arbitration(&data,&remote,&winner)==SD_OK&&winner==0);
 cn_values("arbitration",counterexamples,0x123,11,29);return 0;
}
static int equal(const struct sd_can *a,const struct sd_can *b){
 if(a->identifier!=b->identifier||a->extended!=b->extended||a->remote!=b->remote||a->length!=b->length)return 0;
 for(unsigned i=0;i<a->length;++i)if(a->data[i]!=b->data[i])return 0;
 return 1;
}
/* Re-stuff an altered CRC while retaining every fixed-form bit. The fixture
   changes CRC validity, not an accidental stuff/form boundary. */
static size_t crc_corrupt(const uint8_t *wire,size_t n,uint8_t *bad){
 uint8_t raw[128];size_t used=0;unsigned last=2,run=0;
 for(size_t i=0;i<n-10;++i){
  unsigned b=wire[i];if(run==5){last=b;run=1;continue;}
  raw[used++]=(uint8_t)b;run=b==last?run+1:1;last=b;
 }
 raw[used-1]^=1;size_t count=0;last=2;run=0;
 for(size_t i=0;i<used;++i){unsigned b=raw[i];bad[count++]=(uint8_t)b;run=b==last?run+1:1;last=b;if(run==5){last^=1;bad[count++]=(uint8_t)last;run=1;}}
 for(unsigned i=0;i<10;++i)bad[count++]=wire[n-10+i];
 return count;
}
static int codec(void){
 unsigned frames=0,rejections=0;
 for(unsigned len=0;len<=8;++len)for(unsigned pattern=0;pattern<4;++pattern){
  struct sd_can f={0x123,0,0,(uint8_t)len,{0}};uint8_t bits[160],bad[160];size_t n=0,consumed=99;
  for(unsigned i=0;i<len;++i)f.data[i]=(uint8_t)(pattern==0?0:pattern==1?255:pattern==2?0xa5:i*37+3);
  CHECK(sd_can_encode(&f,bits,sizeof bits,&n)==SD_OK);
  struct cn_stream s={0};struct cn_decoded out={0};
  CHECK(cn_feed(&s,bits,n,&consumed,&out)==CN_FRAME&&consumed==n&&equal(&out.frame,&f)&&out.ack==1);
  s=(struct cn_stream){0};bits[n-9]=0;
  CHECK(cn_feed(&s,bits,n,&consumed,&out)==CN_FRAME&&out.ack==0&&equal(&out.frame,&f));
  ++frames;s=(struct cn_stream){0};out.frame.identifier=0x555;bits[n-10]=0;
  CHECK(cn_feed(&s,bits,n,&consumed,&out)==CN_ERROR&&out.frame.identifier==0x555);++rejections;
  bits[n-10]=1;size_t k=crc_corrupt(bits,n,bad);s=(struct cn_stream){0};
  CHECK(cn_feed(&s,bad,k,&consumed,&out)==CN_ERROR&&out.frame.identifier==0x555);++rejections;
 }
 struct cn_stream s={0};struct cn_decoded out={0};uint8_t invalid[]={0,0,2};size_t count=99;
 CHECK(cn_feed(&s,invalid,3,&count,&out)==CN_INVALID&&count==99&&s.total==0);
 CHECK(cn_feed(&s,0,0,&count,&out)==CN_MORE&&count==0);
 cn_values("codec",frames,rejections,0,8);return 0;
}
static int stream(void){
 struct sd_can f={0,0,0,8,{0}};uint8_t bits[160];size_t n=0;
 CHECK(sd_can_encode(&f,bits,sizeof bits,&n)==SD_OK);
 unsigned packets=0,stuff_boundary=0;
 for(size_t first=0;first<=n;++first)for(size_t second=first;second<=n;++second){
  struct cn_stream s={0};struct cn_decoded out={0};size_t consumed=99;int result;
  result=cn_feed(&s,bits,first,&consumed,&out);CHECK(consumed==first);
  if(s.expect)++stuff_boundary;
  if(result==CN_MORE){result=cn_feed(&s,bits+first,second-first,&consumed,&out);CHECK(consumed==second-first);}
  if(result==CN_MORE){result=cn_feed(&s,bits+second,n-second,&consumed,&out);CHECK(consumed==n-second);}
  CHECK(result==CN_FRAME&&equal(&out.frame,&f));++packets;
 }
 CHECK(stuff_boundary>0);
 struct cn_stream s={0};struct cn_decoded out={0};size_t used=0;
 for(size_t i=0;i<n;++i){int r=cn_feed(&s,bits+i,1,&used,&out);CHECK(r==(i+1==n?CN_FRAME:CN_MORE)&&used==1);}
 CHECK(cn_feed(&s,bits,1,&used,&out)==CN_BUSY);
 cn_values("stream",(unsigned)n,packets,stuff_boundary,1);return 0;
}
static int fault(void){
 struct cn_fault s={0};s.enabled=1;struct cn_effect out={0};struct cn_event e={0};
 e.kind=CN_ARBITRATION_LOSS;e.transmitter=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==0&&s.rec==0);
 s.rec=120;e=(struct cn_event){0};e.kind=CN_RECEIVER_ERROR;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.rec==121&&out.rules==1);
 e.kind=CN_FIRST_DOMINANT;CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.rec==129&&s.state==CN_PASSIVE&&out.before_state==CN_ACTIVE);
 e.kind=CN_RECEIVE_SUCCESS;CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.rec==127&&s.state==CN_ACTIVE);
 s=(struct cn_fault){0};s.enabled=1;e=(struct cn_event){0};e.kind=CN_TRANSMITTER_FLAG;e.transmitter=1;
 for(unsigned i=0;i<16;++i)CHECK(cn_fault(&s,&e,&out)==CN_OK);
 CHECK(s.tec==128&&s.state==CN_PASSIVE);
 e.ack_missing=e.passive_flag_all_recessive=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==128&&!out.rules);
 e.passive_flag_all_recessive=0;CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==136&&out.rules==4);
 e.ack_missing=0;e.prearbitration_stuff_exception=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==136&&!out.rules);
 s=(struct cn_fault){0};s.enabled=1;e=(struct cn_event){0};e.kind=CN_TRANSMITTER_FLAG;e.transmitter=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==8);
 e.kind=CN_BIT_DURING_FLAG;e.active_or_overload=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==16&&out.rules==8);
 e.kind=CN_DOMINANT_RUN;e.value=22;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==32&&out.rules==32);
 s=(struct cn_fault){0};s.enabled=1;s.rec=250;s.state=CN_PASSIVE;e.transmitter=0;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.rec==255);
 e.kind=CN_RECEIVE_SUCCESS;e.active_or_overload=0;e.value=0;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.rec==127);
 s=(struct cn_fault){0};s.enabled=1;e=(struct cn_event){0};e.kind=CN_TRANSMITTER_FLAG;e.transmitter=1;
 for(unsigned i=0;i<32;++i)CHECK(cn_fault(&s,&e,&out)==CN_OK);
 CHECK(s.tec==256&&s.state==CN_OFF);
 CHECK(cn_fault(&s,&e,&out)==CN_BUSY&&s.tec==256);
 e=(struct cn_event){0};e.kind=CN_RESTART;CHECK(cn_fault(&s,&e,&out)==CN_OK);
 e.kind=CN_RECOVERY_BIT;e.value=1;
 for(unsigned i=0;i<1407;++i)CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.state==CN_OFF);
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.state==CN_ACTIVE&&s.tec==0&&s.rec==0);
 e=(struct cn_event){0};e.kind=CN_RECEIVER_ERROR;e.active_or_overload=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.rec==0&&!out.rules);
 e.kind=CN_BIT_DURING_FLAG;CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.rec==8&&out.rules==16);
 /* Isolated transmitter: ACK errors rise only to passive, then exception1
    prevents this particular absent-peer history from reaching bus-off. */
 s=(struct cn_fault){0};s.enabled=1;e=(struct cn_event){0};e.kind=CN_TRANSMITTER_FLAG;e.transmitter=1;
 e.ack_missing=e.passive_flag_all_recessive=1;
 for(unsigned i=0;i<40;++i)CHECK(cn_fault(&s,&e,&out)==CN_OK);
 CHECK(s.tec==128&&s.state==CN_PASSIVE);
 static const unsigned lengths[]={7,8,13,14,21,22};
 static const unsigned active_delta[]={0,0,0,8,8,16},passive_delta[]={0,8,8,8,16,16};
 for(unsigned i=0;i<6;++i)for(unsigned active=0;active<2;++active){
  s=(struct cn_fault){0};s.enabled=1;e=(struct cn_event){0};e.kind=CN_DOMINANT_RUN;e.transmitter=1;e.active_or_overload=active;e.value=lengths[i];
  CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==(active?active_delta[i]:passive_delta[i]));
 }
 s=(struct cn_fault){0};s.enabled=1;s.tec=1;e=(struct cn_event){0};e.kind=CN_TRANSMIT_SUCCESS;e.transmitter=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==0&&out.rules==64);
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==0);
 s=(struct cn_fault){0};s.enabled=1;s.tec=256;s.state=CN_OFF;e=(struct cn_event){0};e.kind=CN_RESTART;
 CHECK(cn_fault(&s,&e,&out)==CN_OK);e.kind=CN_RECOVERY_BIT;e.value=1;
 for(unsigned i=0;i<10;++i)CHECK(cn_fault(&s,&e,&out)==CN_OK);
 e.value=0;CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.recessive==0&&s.idles==0);
 e.value=1;for(unsigned i=0;i<11;++i)CHECK(cn_fault(&s,&e,&out)==CN_OK);
 CHECK(s.idles==1&&s.state==CN_OFF);
 e.kind=CN_RESTART;e.value=0;CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.idles==0);
 s=(struct cn_fault){0};e=(struct cn_event){0};e.kind=CN_TRANSMITTER_FLAG;e.transmitter=1;
 CHECK(cn_fault(&s,&e,&out)==CN_OK&&s.tec==0&&s.state==CN_ACTIVE);
 cn_values("fault",129,128,256,1408);return 0;
}
static int timing(void){
 struct sd_timing t={0};CHECK(sd_can_timing(42000000,6,11,2,1,&t)==SD_OK&&t.rate==500000&&t.btr==0x1a0005);
 CHECK(sd_can_timing(42000000,6,11,2,3,&t)==SD_INVALID);
 struct cn_search r={0};CHECK(cn_search(48000000,500000,3,4,0,&r)==CN_OK&&r.timing.prescaler==6&&r.timing.bs1==11&&r.timing.bs2==4);
 unsigned matches=r.candidates;
 CHECK(cn_search(48000000,500000,3,4,2000,&r)==CN_UNAVAILABLE&&r.candidates==matches);
 CHECK(cn_search(48000000,123457,3,4,0,&r)==CN_UNAVAILABLE);
 CHECK(cn_search(48000000,500000,4,4,0,&r)==CN_INVALID);
 cn_values("timing",500000,0x1a0005,matches,6);return 0;
}
int cn_test(unsigned n){switch(n){case 0:return arbitration();case 1:return codec();case 2:return stream();case 3:return fault();case 4:return timing();default:return 42;}}
