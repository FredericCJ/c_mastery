#include "depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"B03_ASSERT:%s:%s\n",argv[1],#x);return 1;}}while(0)
struct b3_io {long script[8];unsigned cursor;size_t offsets[8];const unsigned char *base;};
static long scripted(void *c,const unsigned char *p,size_t n){struct b3_io *io=c;(void)n;io->offsets[io->cursor]=(size_t)(p-io->base);return io->script[io->cursor++];}
int main(int argc,char **argv){if(argc!=2)return 2;
 if(!strcmp(argv[1],"io")){const unsigned char bytes[]={1,2,3,4,5};struct b3_writer w={bytes,5,0,0};struct b3_io io={{2,-1,-2,1,2},0,{0},bytes};CHECK(b3_pump(&w,scripted,&io,3)==B3_WAIT&&w.position==2&&io.cursor==3);CHECK(b3_pump(&w,scripted,&io,1)==B3_YIELD&&w.position==3);CHECK(b3_pump(&w,scripted,&io,1)==B3_OK&&w.position==5);CHECK(io.offsets[0]==0&&io.offsets[1]==2&&io.offsets[2]==2&&io.offsets[3]==2&&io.offsets[4]==3);CHECK(b3_pump(&w,scripted,&io,4)==B3_OK&&io.cursor==5);}
 else if(!strcmp(argv[1],"cancel")){struct b3_operation o={0,0,0,0};CHECK(b3_start(&o)==B3_OK&&o.generation==1);CHECK(b3_event(&o,1,B3_TIMEOUT)==B3_WAIT&&o.owned==1&&o.reports==0);CHECK(b3_event(&o,1,B3_COMPLETE)==B3_OK&&o.phase==B3_COMPLETED&&o.reports==1);CHECK(b3_event(&o,1,B3_CANCEL_ACK)==B3_STALE&&o.reports==1);CHECK(b3_start(&o)==B3_OK&&o.generation==2);CHECK(b3_event(&o,1,B3_COMPLETE)==B3_STALE&&o.owned==1);CHECK(b3_event(&o,2,B3_CANCEL_ACK)==B3_STALE);CHECK(b3_event(&o,2,B3_TIMEOUT)==B3_WAIT);CHECK(b3_event(&o,2,B3_CANCEL_ACK)==B3_OK&&o.phase==B3_CANCELLED&&o.reports==2);}
 else if(!strcmp(argv[1],"mmio")){struct b3_device d={15,123,0,0};unsigned out=99;CHECK(b3_ack(&d,5)==B3_OK&&d.status==10&&d.writes==1);CHECK(b3_read_data(&d,&out)==B3_WAIT&&out==99&&d.reads==0);d.status|=1;CHECK(b3_read_data(&d,&out)==B3_OK&&out==123&&d.status==10&&d.reads==1);CHECK(b3_ack(&d,16)==B3_INVALID&&d.status==10&&d.writes==1);}
 else if(!strcmp(argv[1],"deadline")){unsigned start,elapsed,delay;int out=77;CHECK(b3_due(4,250,10,&out)==B3_OK&&out==1);CHECK(b3_due(128,0,1,&out)==B3_AMBIGUOUS&&out==1);for(start=0;start<256;start++)for(elapsed=0;elapsed<128;elapsed++)for(delay=0;delay<128;delay++){CHECK(b3_due((uint8_t)(start+elapsed),(uint8_t)start,(uint8_t)delay,&out)==B3_OK);CHECK(out==(elapsed>=delay));}}
 else if(!strcmp(argv[1],"publication")){struct b3_mailbox m;int out=99;b3_mailbox_init(&m);CHECK(b3_take(&m,&out)==B3_WAIT&&out==99);CHECK(b3_publish(&m,7)==B3_OK);CHECK(b3_publish(&m,8)==B3_WAIT);CHECK(b3_take(&m,&out)==B3_OK&&out==7);CHECK(b3_publish(&m,8)==B3_OK);CHECK(b3_take(&m,&out)==B3_OK&&out==8);CHECK(b3_load_order()==memory_order_acquire||b3_load_order()==memory_order_seq_cst);CHECK(b3_store_order()==memory_order_release||b3_store_order()==memory_order_seq_cst);}
 else return 2;
 return 0;
}
