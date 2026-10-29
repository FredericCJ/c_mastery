#include "foundation.h"
#include <string.h>
int puts(const char *);
#define CHECK(x) do{if(!(x)){puts("EF_ASSERT:" #x);return 41;}}while(0)
static void hex(const char *name,const uint8_t *p,size_t n) {
  char b[193];static const char h[]="0123456789abcdef";
  for(size_t i=0;i<n;i++){b[2*i]=h[p[i]>>4];b[2*i+1]=h[p[i]&15];}b[2*n]=0;puts(name);puts(b);
}
static int ledger(void) {
  struct ef_budget b;
  CHECK(ef_budget(1,0,&b)==EF_OK && b.canonical==33 && b.session==40 && b.count==2 && b.last==16 && b.transport==32 && b.network==38 && b.body==45 && b.wire==96);
  CHECK(ef_budget(2,8,&b)==EF_OK && b.canonical==45 && b.session==52 && b.count==3 && b.last==4);
  b.canonical=999;CHECK(ef_budget(1,8,&b)==EF_INVALID && b.canonical==999);
  /* A framed radio's SDU still carries the complete local link body, including
   * version/kind/address/CRC. Physical delimiting/escaping is no longer needed
   * on that selected lower interface; grant/upper admission remains explicit. */
  struct edu_frame f={0,1,2,2,{0x7e,0x7d}},g;uint8_t wire[96];size_t n;
  CHECK(ef_frame_encode(&f,1,wire,sizeof wire,&n)==EF_OK);
  uint8_t body[47];size_t z=0;
  for(size_t i=1;i<n-1;i++){uint8_t c=wire[i];if(c==0x7d)c=wire[++i]^0x20;body[z++]=c;}
  CHECK(z==9 && edu_crc(body,z-2)==((unsigned)body[z-2]*256+body[z-1]));
  /* A complete radio packet is copied into a validated original L2 wrapper;
   * radio framing does not grant a foreign address or application authority. */
  struct ef_framer state={0};unsigned deliveries=0;
  for(size_t i=0;i<n;i++){int r=ef_frame_byte(&state,1,wire[i],0,&g);if(r==1)deliveries++;}
  unsigned deliver=99;CHECK(deliveries==1 && e7_link_admit(g.kind,2,g.destination,0,0,0,&deliver)==E7_REJECTED && deliver==99);
  puts("EF_LEDGER_PASS schema1=33,40,2,16 schema2=45,52,3,4 envelope=80,87,4,15 link=47,96 radio_keeps_address_grant_crc");return 0;
}
static int framing(void) {
  struct edu_frame f={0,1,2,40,{0}},g;uint8_t wire[96],v2[96],stream[240];size_t n,m;
  for(unsigned i=0;i<40;i++)f.payload[i]=(uint8_t)(i%2?0x7d:0x7e);
  CHECK(edu_crc("123456789",9)==0x29b1);
  CHECK(ef_frame_encode(&f,1,wire,sizeof wire,&n)==EF_OK && n<=96);
  CHECK(ef_frame_encode(&f,2,v2,sizeof v2,&m)==EF_OK && m<=96);
  hex("EF_V1",wire,n);hex("EF_V2",v2,m);
  for(unsigned version=1;version<=2;version++) {
    const uint8_t *p=version==1?wire:v2;size_t count=version==1?n:m,z=0;
    stream[z++]=0x7e;for(unsigned i=0;i<48;i++)stream[z++]=0x55;
    stream[z++]=0x7e;stream[z++]=0x7d;stream[z++]=0x7e;
    memcpy(stream+z,p,count);z+=count;
    for(size_t split=0;split<=z;split++) {
      struct ef_framer s={0};unsigned delivered=0;
      for(unsigned chunk=0;chunk<2;chunk++)for(size_t i=chunk?split:0;i<(chunk?z:split);i++) {
        int r=ef_frame_byte(&s,version,stream[i],0,&g);
        if(r==1){delivered++;CHECK(g.length==40 && !memcmp(g.payload,f.payload,40));}
      }
      CHECK(delivered==1);
    }
    struct ef_framer s={0};unsigned delivered=0;
    for(size_t i=0;i<count;i++)if(ef_frame_byte(&s,version,p[i],0,&g)==1)delivered++;
    CHECK(delivered==1);
    for(size_t i=0;i<count;i++)if(ef_frame_byte(&s,version,p[i],i==5,&g)==1)delivered++;
    CHECK(delivered==1);
    for(size_t i=0;i<count;i++)if(ef_frame_byte(&s,version,p[i],0,&g)==1)delivered++;
    CHECK(delivered==2);
    /* Escape an ordinary version byte while retaining its decoded value and
     * valid CRC. The raw legacy decoder permits this; the strict adapter must
     * reject this noncanonical encoding before publication. */
    struct ef_framer bad={0};unsigned bad_deliveries=0;
    CHECK(ef_frame_byte(&bad,version,0x7e,0,&g)==0);
    CHECK(ef_frame_byte(&bad,version,0x7d,0,&g)==0);
    (void)ef_frame_byte(&bad,version,(uint8_t)(version^(version==1?0x20:0x40)),0,&g);
    for(size_t i=2;i<count;i++)if(ef_frame_byte(&bad,version,p[i],0,&g)==1)bad_deliveries++;
    CHECK(bad_deliveries==0);
  }
  struct ef_framer mismatch={0};unsigned count=0;for(size_t i=0;i<m;i++)if(ef_frame_byte(&mismatch,1,v2[i],0,&g)==1)count++;CHECK(!count);
  /* Original local attacker knows the public CRC; this changes the protected
   * payload while preserving a valid frame, without external targets. */
  f.payload[0]=1;CHECK(ef_frame_encode(&f,1,wire,sizeof wire,&n)==EF_OK);
  struct ef_framer altered={0};count=0;for(size_t i=0;i<n;i++)if(ef_frame_byte(&altered,1,wire[i],0,&g)==1)count++;
  CHECK(count==1 && g.payload[0]==1);
  unsigned deliver=99;CHECK(e7_link_admit(0,1,2,1,0,0,&deliver)==E7_REJECTED && deliver==99);
  CHECK(e7_link_admit(0,2,2,0,0,0,&deliver)==E7_REJECTED && deliver==99);
  CHECK(e7_link_admit(0,2,2,1,0,0,&deliver)==E7_OK && deliver==1);
  puts("EF_FRAMING_PASS all_splits_noise_escape_error_two_completions_valid_crc_forgery_no_authentication");return 0;
}
static int router(void) {
  uint8_t input[40],saved[40];size_t n;struct edu_datagram d={17,51,4,3,{7,8,42}},local;
  struct ef_route r={0};uint32_t a=99,b;struct edu_frame f;unsigned session=123;
  CHECK(!edu_network_encode(&d,input,&n));memcpy(saved,input,n);
  CHECK(ef_enqueue(&r,input,n,0,1,1,&a)==EF_OK);memset(input,0xaa,n);
  CHECK(ef_peek(&r,a,&f)==EF_OK && f.source==1 && f.destination==2 && f.length==9 && f.payload[1]==17 && f.payload[2]==51 && f.payload[3]==3 && !memcmp(f.payload+6,d.payload,3));
  CHECK(ef_enqueue(&r,saved,n,0,1,1,&b)==EF_BUSY && session==123);
  CHECK(ef_release(&r,a)==EF_OK);
  CHECK(ef_enqueue(&r,saved,n,0,0,1,&b)==EF_OK && b>a);
  CHECK(ef_release(&r,a)==EF_STALE && r.active);
  CHECK(ef_peek(&r,b,&f)==EF_OK && !memcmp(f.payload+6,d.payload,3));
  CHECK(ef_release(&r,b)==EF_OK);memset(saved,0xbb,n);CHECK(ef_peek(&r,b,&f)==EF_STALE);
  d=(struct edu_datagram){51,17,4,3,{8,7,42}};CHECK(!edu_network_encode(&d,input,&n));
  CHECK(ef_enqueue(&r,input,n,1,1,1,&a)==EF_OK && ef_peek(&r,a,&f)==EF_OK && f.source==2 && f.destination==1 && f.payload[1]==51 && f.payload[2]==17 && f.payload[3]==3);
  CHECK(ef_release(&r,a)==EF_OK);d.hop=1;CHECK(!edu_network_encode(&d,input,&n));
  CHECK(ef_local(input,n,17,&local)==EF_OK && local.hop==1);
  CHECK(ef_enqueue(&r,input,n,1,1,1,&a)==EF_DROP);
  d.hop=4;d.source=34;CHECK(!edu_network_encode(&d,input,&n));CHECK(ef_enqueue(&r,input,n,1,1,1,&a)==EF_DROP);
  d.source=17;d.destination=68;CHECK(!edu_network_encode(&d,input,&n));
  CHECK(ef_enqueue(&r,input,n,0,1,1,&a)==EF_DROP);
  CHECK(ef_enqueue(&r,input,n,0,1,2,&a)==EF_OK && ef_peek(&r,a,&f)==EF_OK && f.payload[2]==68 && f.destination==2 && session==123);
  CHECK(ef_release(&r,a)==EF_OK);input[4]=3;CHECK(ef_enqueue(&r,input,n,0,1,2,&a)==EF_DROP);
  input[4]=4;CHECK(ef_enqueue(&r,input,n-1,0,1,2,&a)==EF_DROP);
  r.generation=UINT32_MAX;CHECK(ef_enqueue(&r,input,n,0,1,2,&a)==EF_FULL);
  puts("EF_ROUTER_PASS copy_poison reverse_next_hop hop1_local borrow_release_generation busy_atomic route68_profile2");return 0;
}
static int codec(void) {
  struct edu_record r={2,42,-4000,1,1,{0},0},out;uint8_t b[80],old[80];size_t n=999;
  memset(b,0xaa,sizeof b);memcpy(old,b,sizeof b);
  CHECK(ef_encode(&r,b,14,&n)==EF_FULL && n==999 && !memcmp(b,old,sizeof b));
  CHECK(ef_encode(&r,b,sizeof b,&n)==EF_OK && n==15 && b[10]==0xf0 && b[11]==0x60);hex("EF_SET",b,n);
  for(unsigned i=0;i<4;i++){r.label[4*i]=0xf0;r.label[4*i+1]=0x9f;r.label[4*i+2]=0x98;r.label[4*i+3]=0x80;}r.label_length=16;
  CHECK(ef_encode(&r,b,sizeof b,&n)==EF_OK && n==33);
  for(size_t split=0;split<=n;split++){uint8_t joined[80];memcpy(joined,b,split);memcpy(joined+split,b+split,n-split);CHECK(!edu_record_decode(joined,n,&out) && out.label_length==16 && out.value==-4000);}
  const uint8_t invalid[][4]={{0xed,0xa0,0x80,0},{0xc0,0x80,0,0},{0xf4,0x90,0x80,0x80},{0xe2,0x82,0,0}};
  const unsigned sizes[]={3,2,4,2};
  for(unsigned i=0;i<4;i++){r.label_length=(uint8_t)sizes[i];memcpy(r.label,invalid[i],sizes[i]);n=999;CHECK(ef_encode(&r,b,sizeof b,&n)==EF_INVALID && n==999);}
  r.label_length=3;memcpy(r.label,"%n%",3);r.value=12501;
  CHECK(ef_encode(&r,b,sizeof b,&n)==EF_OK && !edu_record_decode(b,n,&out) && out.value==12501);
  int16_t set=2200;struct edu_record reply;CHECK(!edu_application(&out,&set,&reply) && set==2200 && reply.verb==4);
  uint8_t bad[80];memcpy(bad,b,n);bad[4]=1;memset(&out,0x5a,sizeof out);struct edu_record before;memcpy(&before,&out,sizeof out);
  CHECK(edu_record_decode(bad,n,&out) && !memcmp(&out,&before,sizeof out));
  memcpy(bad,b,n);bad[1]=2;CHECK(edu_record_decode(bad,n,&out) && !memcmp(&out,&before,sizeof out));
  struct ef_wide w={0},v;w.base=r;w.value=INT32_MIN;w.extra_length=8;memset(w.extra,0x6a,8);
  CHECK(ef_wide_encode(&w,b,sizeof b,&n)==EF_OK && !ef_wide_decode(b,n,&v) && v.value==INT32_MIN && v.extra_length==8);
  CHECK(edu_record_decode(b,n,&out));hex("EF_WIDE",b,n);
  w.value=(int64_t)INT32_MAX+1;n=999;CHECK(ef_wide_encode(&w,b,sizeof b,&n)==EF_INVALID && n==999);
  w.value=INT32_MAX;CHECK(ef_wide_encode(&w,b,sizeof b,&n)==EF_OK && ef_wide_decode(b,n,&v)==EF_OK && v.value==INT32_MAX);
  /* Translation affects only label byte/fragment count, never numeric unit or
   * the separate token/route authority. ASCII1 versus four4-byte codepoints. */
  r.value=-4000;r.label_length=1;r.label[0]='A';CHECK(ef_encode(&r,b,sizeof b,&n)==EF_OK && n==18 && (n+7+23)/24==2);
  r.label_length=16;for(unsigned i=0;i<4;i++){r.label[4*i]=0xf0;r.label[4*i+1]=0x9f;r.label[4*i+2]=0x98;r.label[4*i+3]=0x80;}
  CHECK(ef_encode(&r,b,sizeof b,&n)==EF_OK && n==33 && !edu_record_decode(b,n,&out) && out.has_unit && out.value==-4000);
  puts("EF_CODEC_PASS f060 capacity_atomic UTF8_16 widened_i32 schema2_no_v1_authority display_octets_only");return 0;
}
int ef_tests(unsigned n){return n==0?ledger():n==1?framing():n==2?router():n==3?codec():41;}
