#include "transport.h"
#include <string.h>
int puts(const char *);
#define CHECK(x) do{if(!(x)){puts("ET_ASSERT:" #x);return 41;}}while(0)
static void hex(const char *tag,const uint8_t *p,size_t n){char b[81];const char *digits="0123456789abcdef";for(size_t i=0;i<n;i++){b[2*i]=digits[p[i]>>4];b[2*i+1]=digits[p[i]&15];}b[2*n]=0;puts(tag);puts(b);}
static int reassembly(void){
 EtTx tx={0};EtRx rx={0};uint8_t data[120],wire[40],first[40],last[40],ack[40],out[120];size_t n=0,a=0,z=99,fn=0,ln=0;uint32_t token=0;
 for(unsigned i=0;i<120;i++)data[i]=(uint8_t)i;
 CHECK(et_tx_open(&tx,1,1,17)==E7_OK && et_rx_open(&rx,1,1,51)==E7_OK);
 CHECK(et_begin(&tx,1,data,25,0)==E7_OK && et_offer(&tx,0,wire,sizeof wire,&n,&token)==E7_OK && n==38);
 hex("ET_PACKET_V1",wire,n);
 EtRx saved;memcpy(&saved,&rx,sizeof rx);memset(ack,0xa5,sizeof ack);a=99;
 CHECK(et_receive(&rx,wire,n,0,ack,13,&a)==E7_FULL && a==99 && ack[0]==0xa5 && memcmp(&rx,&saved,sizeof rx)==0);
 memcpy(first,wire,n);fn=n;CHECK(et_accept(&tx,1,token,1,0)==E7_OK);
 CHECK(et_receive(&rx,wire,n,0,ack,sizeof ack,&a)==E7_OK && rx.inner.length==24 && !rx.inner.ready && a==14);
 CHECK(et_peek(&rx,out,sizeof out,&z)==E7_WAIT && z==99);
 CHECK(et_receive(&rx,first,fn,99,ack,sizeof ack,&a)==E7_OK && rx.started==0 && rx.inner.length==24);
 CHECK(et_ack(&tx,ack,a,99)==E7_OK);
 CHECK(et_offer(&tx,99,wire,sizeof wire,&n,&token)==E7_OK && n==15);memcpy(last,wire,n);ln=n;
 CHECK(et_accept(&tx,1,token,1,99)==E7_OK);
 CHECK(et_receive(&rx,wire,n,99,ack,sizeof ack,&a)==ET_COMPLETE && rx.inner.publications==1 && ack[13]==0);
 CHECK(et_ack(&tx,ack,a,99)==E7_OK && !tx.inner.active && tx.peer_credit==0);
 CHECK(et_receive(&rx,last,ln,100,ack,sizeof ack,&a)==E7_OK && rx.inner.publications==1 && ack[13]==0);
 last[ln-1]^=1;memcpy(&saved,&rx,sizeof rx);memset(ack,0xa5,sizeof ack);a=99;
 CHECK(et_receive(&rx,last,ln,101,ack,sizeof ack,&a)==E7_REJECTED && ack[0]==0xa5 && a==99 && memcmp(&rx,&saved,sizeof rx)==0);
 CHECK(et_peek(&rx,out,24,&z)==E7_FULL && z==99);
 CHECK(et_peek(&rx,out,sizeof out,&z)==E7_OK && z==25 && !memcmp(out,data,25));
 CHECK(et_release(&rx,2,1)==E7_STALE && rx.inner.ready);
 CHECK(et_release(&rx,1,1)==E7_OK);
 /* Fresh120-byte changed-condition control and a held upper SDU. */
 EtTx wide={0};EtRx held={0};CHECK(et_tx_open(&wide,1,1,17)==0&&et_rx_open(&held,1,1,51)==0&&et_begin(&wide,1,data,120,0)==0);
 for(unsigned i=0;i<5;i++){
  CHECK(et_offer(&wide,i,wire,sizeof wire,&n,&token)==0&&n==38&&wire[11]==i&&wire[12]==5);
  CHECK(et_accept(&wide,1,token,1,i)==0);
  CHECK(et_receive(&held,wire,n,i,ack,sizeof ack,&a)==(i==4?ET_COMPLETE:E7_OK));
  CHECK(et_ack(&wide,ack,a,i)==0);
 }
 CHECK(held.inner.ready&&held.inner.length==120&&held.inner.publications==1&&wide.peer_credit==0);
 CHECK(et_peek(&held,out,sizeof out,&z)==0&&z==120&&!memcmp(data,out,120));
 CHECK(et_begin(&wide,2,data,1,10)==0&&wide.inner.blocked);
 CHECK(et_offer(&wide,109,wire,sizeof wire,&n,&token)==E7_WAIT);
 CHECK(et_offer(&wide,110,wire,sizeof wire,&n,&token)==0&&n==14&&wire[6]==3);
 CHECK(et_accept(&wide,1,token,1,110)==0&&et_receive(&held,wire,n,110,ack,sizeof ack,&a)==0&&ack[13]==0);
 CHECK(et_ack(&wide,ack,a,110)==0&&wide.inner.blocked_since==10&&held.inner.ready);
 for(unsigned profile=1;profile<=2;profile++)for(unsigned length=1;length<=120;length++){
  EtTx t={0};EtRx r={0};CHECK(et_tx_open(&t,profile,1,17)==0&&et_rx_open(&r,profile,1,51)==0&&et_begin(&t,1,data,length,0)==0);
  unsigned count=(length+23)/24;
  for(unsigned j=0;j<count;j++){
   CHECK(et_offer(&t,j,wire,sizeof wire,&n,&token)==0&&et_accept(&t,1,token,1,j)==0);
   CHECK(et_receive(&r,wire,n,j,ack,sizeof ack,&a)==(j+1==count?ET_COMPLETE:E7_OK));
   CHECK(et_receive(&r,wire,n,j,ack,sizeof ack,&a)==E7_OK&&et_ack(&t,ack,a,j)==0);
  }
  CHECK(et_peek(&r,out,sizeof out,&z)==0&&z==length&&!memcmp(out,data,length)&&r.inner.publications==1);
 }
 puts("ET_LENGTHS 240 full_messages two_profiles duplicate_each_fragment");
 puts("ET_REASSEMBLY_PASS 24+1 5x24 one_publication held_credit0");return 0;
}
static int retry(void){
 EtTx tx={0};uint8_t data[1]={7},wire[40];size_t n;uint32_t token;
 CHECK(et_tx_open(&tx,1,1,17)==0&&et_begin(&tx,1,data,1,0)==0);
 for(unsigned now=0;now<250;now+=25){CHECK(et_offer(&tx,now,wire,sizeof wire,&n,&token)==0);CHECK(et_accept(&tx,1,token,0,now)==E7_BUSY&&!tx.inner.waiting&&tx.inner.due==0&&tx.inner.retries==0);}
 CHECK(et_offer(&tx,250,wire,sizeof wire,&n,&token)==0&&et_accept(&tx,1,token,1,250)==0&&tx.inner.due==350);
 CHECK(et_offer(&tx,349,wire,sizeof wire,&n,&token)==E7_WAIT);
 for(unsigned k=0;k<3;k++){
  unsigned due=350+k*100;
  CHECK(et_offer(&tx,due,wire,sizeof wire,&n,&token)==0&&et_accept(&tx,1,token,0,due)==E7_BUSY&&tx.inner.retries==k&&tx.inner.due==due);
  CHECK(et_offer(&tx,due,wire,sizeof wire,&n,&token)==0&&et_accept(&tx,1,token,1,due)==0&&tx.inner.retries==k+1&&tx.inner.due==due+100);
 }
 CHECK(et_offer(&tx,649,wire,sizeof wire,&n,&token)==E7_WAIT);
 CHECK(et_offer(&tx,650,wire,sizeof wire,&n,&token)==E7_TIMEOUT&&!tx.inner.active);
 puts("ET_RETRY_PASS accepted250 350 450 550 terminal650 no_offer_clock");return 0;
}
static int expiry(void){
 EtTx tx={0};EtRx rx={0};uint8_t data[25]={0},wire[40],old[40],ack[40],out[120];size_t n=0,a=0,z=99;uint32_t token;
 CHECK(et_tx_open(&tx,2,1,17)==0&&et_rx_open(&rx,2,1,51)==0&&et_begin(&tx,1,data,25,0)==0);
 CHECK(et_offer(&tx,0,wire,sizeof wire,&n,&token)==0&&n==40&&wire[4]==0x84);memcpy(old,wire,n);
 hex("ET_PACKET_V2",wire,n);
 CHECK(et_accept(&tx,1,token,1,0)==0&&et_receive(&rx,wire,n,0,ack,sizeof ack,&a)==0);
 CHECK(et_receive(&rx,wire,n,499,ack,sizeof ack,&a)==0&&rx.started==0);
 CHECK(et_expire(&rx,500)==E7_TIMEOUT&&!rx.inner.active&&rx.inner.length==0&&rx.retired==1);
 CHECK(et_receive(&rx,wire,n,501,ack,sizeof ack,&a)==E7_STALE);
 CHECK(et_peek(&rx,out,sizeof out,&z)==E7_WAIT&&z==99);
 CHECK(et_rx_open(&rx,2,2,51)==0);
 CHECK(et_receive(&rx,old,n,502,ack,sizeof ack,&a)==E7_STALE&&!rx.inner.active);
 EtRx forged={0};CHECK(et_rx_open(&forged,2,2,51)==0);old[6]=0;old[7]=2;
 CHECK(et_receive(&forged,old,n,502,ack,sizeof ack,&a)==0&&forged.inner.active);
 EtTx fresh={0};CHECK(et_tx_open(&fresh,2,2,17)==0&&et_begin(&fresh,1,data,25,502)==0);
 CHECK(et_offer(&fresh,502,wire,sizeof wire,&n,&token)==0&&et_receive(&rx,wire,n,502,ack,sizeof ack,&a)==0&&rx.inner.active);
 /* Original v1 has no epoch: numeric ID reuse cannot provide this guarantee.
    Current wrapper forbids reopening v1; the explicit protocol2 wire is tested. */
 EtTx original={0};CHECK(et_tx_open(&original,1,1,17)==0&&et_tx_open(&original,1,2,17)==E7_STALE);
 EtTx blocked={0};EtRx complete={0};CHECK(et_tx_open(&blocked,1,1,17)==0&&et_rx_open(&complete,1,1,51)==0&&et_begin(&blocked,1,data,1,0)==0);
 CHECK(et_offer(&blocked,0,wire,sizeof wire,&n,&token)==0&&et_accept(&blocked,1,token,1,0)==0&&et_receive(&complete,wire,n,0,ack,sizeof ack,&a)==ET_COMPLETE&&et_ack(&blocked,ack,a,0)==0);
 CHECK(et_begin(&blocked,2,data,25,1)==0);
 for(unsigned now=101;now<501;now+=100){CHECK(et_offer(&blocked,now,wire,sizeof wire,&n,&token)==0&&et_accept(&blocked,1,token,1,now)==0&&et_receive(&complete,wire,n,now,ack,sizeof ack,&a)==0&&et_ack(&blocked,ack,a,now)==0&&blocked.inner.blocked_since==1);}
 CHECK(et_offer(&blocked,501,wire,sizeof wire,&n,&token)==E7_TIMEOUT&&!blocked.inner.active&&complete.inner.ready&&complete.inner.publications==1);
 EtTx exhausted={0};EtRx end={0};CHECK(et_tx_open(&exhausted,2,65535,17)==0&&et_rx_open(&end,2,65535,51)==0&&et_begin(&exhausted,65535,data,1,0)==0);
 CHECK(et_offer(&exhausted,0,wire,sizeof wire,&n,&token)==0&&et_accept(&exhausted,65535,token,1,0)==0&&et_receive(&end,wire,n,0,ack,sizeof ack,&a)==ET_COMPLETE&&et_ack(&exhausted,ack,a,0)==0);
 CHECK(et_begin(&exhausted,0,data,1,1)==E7_INVALID&&et_tx_open(&exhausted,2,1,17)==E7_STALE);
 puts("ET_EXPIRY_PASS deadline500 no_duplicate_renewal protocol2_epoch held_SDU_survives_sender_failure");return 0;
}
int et_tests(unsigned n){return n==0?reassembly():n==1?retry():n==2?expiry():41;}
