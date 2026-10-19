#include "pipeline.h"
#include <limits.h>
#include <string.h>
int puts(const char *);
#define CHECK(x) do{if(!(x)){puts("EW_ASSERT:" #x);return 41;}}while(0)
typedef struct {uint32_t r[16],due;unsigned de,pending,byte,edges,writes,dropped,corrupt,cut,split_after,pauses,premature;uint32_t hold_until;} Uart;
typedef struct {EwNode a,r,b;Uart u[4];uint32_t now;unsigned collisions,overruns;EduFramer monitor[4];uint8_t current[4][96],capture[4][96];size_t used[4],length[4];} World;
static World world;
static uint32_t read_io(void *c,unsigned k){Uart *u=c;uint32_t v=k==PU_SOURCE?1:u->r[k];if(k==PU_DR)u->r[PU_SR]&=~47u;if(k==PU_SR&&world.now<u->hold_until)v&=~PU_TXE;return v;}
static void write_io(void *c,unsigned k,uintptr_t v){Uart *u=c;if(k==PU_SR)u->r[k]&=(uint32_t)v;else u->r[k]=(uint32_t)v;
 if(k==PU_DR){u->pending=1;u->byte=(unsigned)v;u->due=world.now+87;u->writes++;u->r[PU_SR]&=~(PU_TXE|PU_TC);}}
static void drive(void *c,unsigned on){Uart *u=c;u->de=on;u->edges++;}
static int init(unsigned profile){
 memset(&world,0,sizeof world);R5Io io[4];for(unsigned i=0;i<4;i++){world.u[i].r[PU_SR]=PU_TXE|PU_TC;io[i]=(R5Io){{&world.u[i],read_io,write_io},&world.u[i],drive};}
 PuClock clock={42000000,1,1,1,1,0};
 CHECK(ew_init(&world.a,17,profile,1,&io[0],&clock,0)==0);
 CHECK(ew_init(&world.r,34,profile,1,&io[1],&clock,0)==0);
 CHECK(ew_init(&world.b,51,profile,1,&io[3],&clock,0)==0);return 0;
}
static void observe(unsigned i,uint8_t byte){
 size_t *used=&world.used[i];if(*used<96)world.current[i][(*used)++]=byte;EduFrame f;int r=edu_frame_byte(&world.monitor[i],byte,0,&f);
 if(r==1&&f.kind==0&&f.length>=29&&f.payload[4]==4&&f.payload[14]==3&&f.payload[27]==0&&f.payload[28]==42&&!world.length[i]){memcpy(world.capture[i],world.current[i],*used);world.length[i]=*used;}
 if(world.u[i].split_after&&*used==world.u[i].split_after&&world.monitor[i].length>11&&world.monitor[i].bytes[11]==1){world.u[i].hold_until=world.now+370;world.u[i].split_after=0;world.u[i].pauses++;world.u[i].premature=world.b.endpoint.effects;}
 if(byte==0x7e){world.current[i][0]=byte;*used=1;}
}
static void hex(const uint8_t *p,size_t n){char text[193];const char digits[]="0123456789abcdef";for(size_t i=0;i<n;i++){text[2*i]=digits[p[i]>>4];text[2*i+1]=digits[p[i]&15];}text[2*n]=0;puts(text);}
static void arrival(unsigned which,unsigned byte,unsigned error){Uart *u=&world.u[which];if(u->r[PU_SR]&PU_RXNE){u->r[PU_SR]|=PU_ORE;world.overruns++;}u->r[PU_DR]=byte;u->r[PU_SR]|=PU_RXNE|error;}
static int tick(void){
 world.now+=10;
 for(unsigned i=0;i<4;i++)if(world.u[i].pending&&world.now>=world.u[i].due){Uart *u=&world.u[i];u->pending=0;u->r[PU_SR]|=PU_TXE|PU_TC;arrival(i,u->byte,0);observe(i,(uint8_t)u->byte);
  if(u->cut)u->dropped++;else{unsigned v=u->byte;if(u->corrupt&&!--u->corrupt)v^=1;arrival(i^1u,v,0);}}
 if((world.u[0].de&&world.u[1].de)||(world.u[2].de&&world.u[3].de))world.collisions++;
 CHECK(ew_tick(&world.a,world.now)==0&&ew_tick(&world.r,world.now)==0&&ew_tick(&world.b,world.now)==0);return 0;
}
static int event(EwNode *s,EsEvent *e){uint32_t end=world.now+2000000;while(world.now<end){if(s->event_ready){CHECK(ew_take(s,e)==0);return 0;}CHECK(tick()==0);}CHECK(0);return 41;}
static int done(EwNode *s){uint32_t end=world.now+2000000;while(world.now<end){if(!s->endpoint.outgoing_kind&&!s->endpoint.tx.inner.active)return 0;CHECK(tick()==0);}CHECK(0);return 41;}
static int open_pair(void){
 uint8_t params[3]={1,1,17};EsEvent e;CHECK(es_emit(&world.a.endpoint,1,7,1,0,params,3,1,world.now/1000)==0);
 CHECK(event(&world.b,&e)==0&&e.type==1&&done(&world.a)==0);
 CHECK(es_emit(&world.b.endpoint,2,7,1,0,params,3,1,world.now/1000)==0);
 CHECK(event(&world.a,&e)==0&&e.type==2&&done(&world.b)==0);return 0;
}
static int handoff(EwNode *from,EwNode *to,uint16_t id){uint8_t next=(uint8_t)to->node;EsEvent e;CHECK(es_emit(&from->endpoint,5,7,1,0,&next,1,id,world.now/1000)==0);CHECK(event(to,&e)==0&&e.type==5&&done(from)==0);return 0;}
static int record(unsigned verb,unsigned req,int value,uint8_t *p,size_t *n){EduRecord r={.verb=(uint8_t)verb,.request=(uint16_t)req,.value=(int16_t)value,.has_value=verb!=1,.has_unit=verb!=1};return ef_encode(&r,p,80,n);}
static int request(unsigned verb,unsigned req,int value,unsigned cp,unsigned transfer,EsEvent *e){uint8_t p[80];size_t n;CHECK(record(verb,req,value,p,&n)==0);CHECK(es_emit(&world.a.endpoint,3,7,1,(uint16_t)cp,p,n,(uint16_t)transfer,world.now/1000)==0);CHECK(event(&world.b,e)==0&&done(&world.a)==0);return 0;}
static int response(const EsEvent *request,unsigned cp,unsigned transfer){EsEvent e;CHECK(es_emit(&world.b.endpoint,3,7,1,(uint16_t)cp,request->reply,request->length,(uint16_t)transfer,world.now/1000)==0);CHECK(event(&world.a,&e)==0&&done(&world.b)==0);return 0;}
static int exchange(void){
 CHECK(init(1)==0&&open_pair()==0);EsEvent e,duplicate;CHECK(request(2,42,-4000,1,2,&e)==0);
 CHECK(world.b.endpoint.setpoint==-4000&&world.b.endpoint.effects==1&&e.length==15);
 CHECK(es_emit(&world.b.endpoint,3,7,1,1,e.reply,e.length,2,world.now/1000)==E7_BUSY);
 CHECK(request(2,42,-4000,1,3,&duplicate)==0&&duplicate.duplicate&&world.b.endpoint.effects==1&&e.length==duplicate.length&&!memcmp(e.reply,duplicate.reply,e.length));
 CHECK(handoff(&world.a,&world.b,4)==0&&response(&duplicate,1,2)==0);
 CHECK(world.a.endpoint.response.request==42&&world.a.endpoint.response.value==-4000&&world.a.endpoint.response.verb==3);
 CHECK(handoff(&world.b,&world.a,3)==0);
 CHECK(request(2,43,12501,2,5,&e)==0&&world.b.endpoint.effects==1&&world.b.endpoint.setpoint==-4000);
 CHECK(handoff(&world.a,&world.b,6)==0&&response(&e,2,4)==0);
 CHECK(world.a.endpoint.response.verb==4&&world.a.endpoint.response.request==43&&world.a.endpoint.response.value==-4000);
 CHECK(handoff(&world.b,&world.a,5)==0&&request(1,44,0,3,7,&e)==0);
 CHECK(handoff(&world.a,&world.b,8)==0&&response(&e,3,6)==0);
 CHECK(world.a.endpoint.response.verb==3&&world.a.endpoint.response.request==44&&world.a.endpoint.response.value==1800&&world.b.endpoint.setpoint==-4000&&world.b.endpoint.effects==1);
 CHECK(!world.r.endpoint.self&&!world.r.endpoint.core.open&&world.r.forwarded>20&&world.r.port[0].frames&&world.r.port[1].frames);
 CHECK(!world.collisions&&!world.overruns);
 CHECK(ew_physical_verdict(1,0)==EW_BUSY);
 puts("EW_EXCHANGE_VALUES -4000 1800 1 3 0");
 puts("EW_EXCHANGE_PASS actual_three_nodes_four_UART_owners_two_segments");return 0;
}
static int identities(void){
 CHECK(init(1)==0&&open_pair()==0);EsEvent e;CHECK(request(2,42,-4000,1,2,&e)==0);
 CHECK(handoff(&world.a,&world.b,3)==0&&response(&e,1,2)==0);
 for(unsigned i=0;i<4;i++){CHECK(world.length[i]>2);EduFramer parser={0};EduFrame f;int got=0;for(size_t k=0;k<world.length[i];k++)if(edu_frame_byte(&parser,world.capture[i][k],0,&f)==1)got++;
  CHECK(got==1&&f.payload[3]==(i==0||i==3?4:3));CHECK(f.source==(i==0||i==2?1:2)&&f.destination==(i==0||i==2?2:1));
  const char *names[]={"EW_WIRE0","EW_WIRE1","EW_WIRE2","EW_WIRE3"};puts(names[i]);hex(world.capture[i],world.length[i]);}
 CHECK(!world.collisions&&!world.overruns);
 /* A real quiescent namespace restart retains only session/application RAM. */
 CHECK(init(2)==0&&open_pair()==0&&request(2,42,-4000,1,2,&e)==0);
 CHECK(ew_rebind(&world.a,2)==EW_BUSY);
 unsigned pa=0,pr=0,pb=0;uint32_t limit=world.now+500000;
 while(world.now<limit){if(!pa)pa=ew_pause(&world.a)==0;if(!pr)pr=ew_pause(&world.r)==0;if(!pb)pb=ew_pause(&world.b)==0;if(pa&&pr&&pb&&ew_drained(&world.a)&&ew_drained(&world.r)&&ew_drained(&world.b))break;CHECK(tick()==0);}
 CHECK(pa&&pr&&pb&&ew_drained(&world.a)&&ew_drained(&world.r)&&ew_drained(&world.b));
 for(unsigned i=0;i<4;i++)CHECK(!world.u[i].pending&&!world.u[i].de);
 CHECK(ew_rebind(&world.a,2)==0&&ew_rebind(&world.b,2)==0&&world.b.endpoint.core.receipt[0].checkpoint==1&&world.b.endpoint.effects==1);
 CHECK(ew_run(&world.a)==0&&ew_run(&world.r)==0&&ew_run(&world.b)==0);
 CHECK(es_emit(&world.a.endpoint,4,7,1,1,0,0,1,world.now/1000)==0&&event(&world.b,&e)==0&&e.type==4&&done(&world.a)==0);
 /* Token/request IDs are new L4 IDs; reply still uses B checkpoint1. */
 CHECK(handoff(&world.a,&world.b,2)==0);uint8_t reply[80];size_t n;CHECK(record(3,42,-4000,reply,&n)==0);e.length=n;memcpy(e.reply,reply,n);
 CHECK(response(&e,1,1)==0&&world.a.endpoint.response.request==42&&world.a.endpoint.response.value==-4000);
 puts("EW_IDENTITIES_VALUES 17 51 7 8 42 1 4 3");puts("EW_IDENTITIES_PASS reverse_wire_and_quiescent_transport_rebind");return 0;
}
static int faults(void){
 CHECK(init(1)==0&&open_pair()==0);EsEvent e;
 /* Disconnect reverse left segment, without changing any device-ready bit. */
 world.u[0].split_after=17;uint8_t data[80];size_t n;CHECK(record(2,42,-4000,data,&n)==0);
 CHECK(es_emit(&world.a.endpoint,3,7,1,1,data,n,2,world.now/1000)==0&&event(&world.b,&e)==0&&world.b.endpoint.effects==1);
 world.u[1].cut=1;uint32_t until=world.now+120000;while(world.now<until)CHECK(tick()==0);
 CHECK(world.u[1].dropped&&world.a.endpoint.tx.inner.active&&world.b.endpoint.effects==1&&world.u[0].pauses==1&&!world.u[0].premature);
 world.u[1].cut=0;CHECK(done(&world.a)==0&&world.b.endpoint.effects==1);
 /* A corrupted/truncated DATA cannot produce a second or partial request. */
 world.u[0].corrupt=12;CHECK(record(1,43,0,data,&n)==0);
 CHECK(es_emit(&world.a.endpoint,3,7,1,2,data,n,3,world.now/1000)==0&&event(&world.b,&e)==0&&done(&world.a)==0&&world.b.endpoint.effects==1);
 /* Full valid lower framing carrying wrong-hop/route datagrams. */
 uint8_t packet_bytes[40];size_t z;EduDatagram d={.source=17,.destination=51,.hop=1,.length=1,.payload={1}};
 CHECK(edu_network_encode(&d,packet_bytes,&z)==0);EduFrame f={.source=1,.destination=2,.length=(uint8_t)z};memcpy(f.payload,packet_bytes,z);
 until=world.now+500000;while(world.a.port[0].queued&&world.now<until)CHECK(tick()==0);
 uint32_t dropped=world.r.route_drops;CHECK(ew_queue(&world.a,0,&f)==0);
 until=world.now+100000;while(world.now<until)CHECK(tick()==0);CHECK(world.r.route_drops>dropped&&world.b.endpoint.effects==1);
 d.hop=4;d.destination=68;CHECK(edu_network_encode(&d,packet_bytes,&z)==0);memcpy(f.payload,packet_bytes,z);dropped=world.r.route_drops;CHECK(ew_queue(&world.a,0,&f)==0);
 until=world.now+100000;while(world.now<until)CHECK(tick()==0);CHECK(world.r.route_drops>dropped);
 /* Valid L4/L5 identity with malformed TLV reaches only its responsible boundary. */
 uint8_t malformed[]={3,0,7,0,1,0,3,1,1,1,2,0x7e};CHECK(et_begin(&world.a.endpoint.tx,4,malformed,sizeof malformed,world.now/1000)==0);
 uint32_t upper=world.b.upper_drops;until=world.now+500000;while(world.now<until&&world.b.upper_drops==upper)CHECK(tick()==0);
 CHECK(world.b.upper_drops>upper&&world.b.endpoint.core.receipt[0].checkpoint==2&&world.b.endpoint.effects==1);
 CHECK(!world.collisions&&!world.overruns&&ew_physical_verdict(1,0)==EW_BUSY);
 puts("EW_FAULTS_VALUES 1 2 0 0");puts("EW_FAULTS_PASS disconnect_retry_corruption_route_and_upper_boundaries");return 0;
}
static int adversarial(void){
 CHECK(init(1)==0);uint8_t badopen[]={1,0,7,0,1,0,0,1,2,17};
 CHECK(et_begin(&world.a.endpoint.tx,1,badopen,sizeof badopen,0)==0);
 uint32_t end=world.now+500000;while(world.now<end&&!world.b.upper_drops)CHECK(tick()==0);
 CHECK(world.b.upper_drops==1&&!world.b.endpoint.core.open&&!world.b.endpoint.effects);
 CHECK(es_emit(&world.b.endpoint,7,7,1,0,0,0,1,world.now/1000)==0&&done(&world.b)==0);
 /* New independent controls establish a valid dialogue before token attack. */
 CHECK(init(1)==0&&open_pair()==0);uint8_t reply[80],violation[87]={3,0,7,0,1,0,1};size_t n;
 CHECK(record(3,42,1800,reply,&n)==0);memcpy(violation+7,reply,n);
 CHECK(et_begin(&world.b.endpoint.tx,2,violation,n+7,world.now/1000)==0);
 end=world.now+500000;while(world.now<end&&!world.a.upper_drops)CHECK(tick()==0);
 CHECK(world.a.upper_drops==1&&!world.a.endpoint.rx.inner.ready&&!world.a.endpoint.responses&&world.a.endpoint.core.receipt[1].checkpoint==0);
 /* A full upper event slot holds the next complete SDU and advertises credit0. */
 CHECK(init(1)==0&&open_pair()==0);uint8_t request_bytes[80];CHECK(record(1,41,0,request_bytes,&n)==0);
 CHECK(es_emit(&world.a.endpoint,3,7,1,1,request_bytes,n,2,world.now/1000)==0);
 end=world.now+500000;while(world.now<end&&!world.b.event_ready)CHECK(tick()==0);CHECK(world.b.event_ready&&done(&world.a)==0);
 CHECK(record(1,42,0,request_bytes,&n)==0&&es_emit(&world.a.endpoint,3,7,1,2,request_bytes,n,3,world.now/1000)==0);
 end=world.now+500000;while(world.now<end&&!world.b.endpoint.rx.inner.ready)CHECK(tick()==0);CHECK(world.b.endpoint.rx.inner.ready&&done(&world.a)==0);
 CHECK(record(1,43,0,request_bytes,&n)==0&&es_emit(&world.a.endpoint,3,7,1,3,request_bytes,n,4,world.now/1000)==0);
 end=world.now+200000;while(world.now<end)CHECK(tick()==0);
 CHECK(world.a.endpoint.tx.inner.blocked&&world.a.endpoint.tx.inner.index==0&&world.b.endpoint.core.receipt[0].checkpoint==1);
 EsEvent e;CHECK(ew_take(&world.b,&e)==0&&event(&world.b,&e)==0&&e.checkpoint==2&&event(&world.b,&e)==0&&e.checkpoint==3&&done(&world.a)==0);
 CHECK(world.b.endpoint.effects==0&&world.b.endpoint.core.receipt[0].checkpoint==3);
 /* A quiescent peer reboot loses L5 RAM despite a valid new transport epoch. */
 CHECK(init(2)==0&&open_pair()==0&&request(2,42,-4000,1,2,&e)==0);
 unsigned pa=0,pr=0,pb=0;end=world.now+500000;
 while(world.now<end){if(!pa)pa=ew_pause(&world.a)==0;if(!pr)pr=ew_pause(&world.r)==0;if(!pb)pb=ew_pause(&world.b)==0;if(pa&&pr&&pb&&ew_drained(&world.a)&&ew_drained(&world.r)&&ew_drained(&world.b))break;CHECK(tick()==0);}
 CHECK(pa&&pr&&pb&&ew_drained(&world.a)&&ew_drained(&world.r)&&ew_drained(&world.b));
 CHECK(ew_rebind(&world.a,2)==0);memset(&world.b,0,sizeof world.b);memset(&world.u[3],0,sizeof world.u[3]);world.u[3].r[PU_SR]=PU_TXE|PU_TC;
 R5Io io={{&world.u[3],read_io,write_io},&world.u[3],drive};PuClock clock={42000000,1,1,1,1,0};
 CHECK(ew_init(&world.b,51,2,2,&io,&clock,world.now)==0&&ew_run(&world.a)==0&&ew_run(&world.r)==0);
 CHECK(es_emit(&world.a.endpoint,4,7,1,1,0,0,1,world.now/1000)==0);
 end=world.now+500000;while(world.now<end&&!world.b.upper_drops)CHECK(tick()==0);
 CHECK(world.b.upper_drops==1&&!world.b.endpoint.core.open&&!world.b.endpoint.effects&&done(&world.a)==0);
 puts("EW_ADVERSARIAL_VALUES 0 0 3 0");puts("EW_ADVERSARIAL_PASS negotiation_token_and_complete_message_credit");return 0;
}
int ew_tests(unsigned n){return n==0?exchange():n==1?identities():n==2?faults():n==3?adversarial():41;}
