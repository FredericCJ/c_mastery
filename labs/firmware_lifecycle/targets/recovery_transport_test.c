#include "recovery_transport.h"
#include "monocypher-ed25519.h"
#include <stdio.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"RT_CHECK:%d:%s\n",__LINE__,#x);return __LINE__;}}while(0)
#define U 0x40004800u
struct f4_m {uint32_t address[32],value[32];unsigned n,reads,dr_reads,tx,clear,flush,ack,tc;unsigned char byte;};
static uint32_t rd(void*p,uint32_t a){struct f4_m*m=p;if(a==U+0x1Cu){++m->reads;return m->ack;}if(a==U+0x24u){++m->dr_reads;return m->byte;}for(unsigned i=0;i<m->n;++i)if(m->address[i]==a)return m->value[i];return 0;}
static void putreg(struct f4_m*m,uint32_t a,uint32_t v){for(unsigned i=0;i<m->n;++i)if(m->address[i]==a){m->value[i]=v;return;}m->address[m->n]=a;m->value[m->n++]=v;}
static void wr(void*p,uint32_t a,uint32_t v){struct f4_m*m=p;if(a==U+0x28u){++m->tx;m->byte=(unsigned char)v;m->ack=m->tc?64u:128u;}else if(a==U+0x20u)m->clear=v;else if(a==U+0x18u)m->flush=v;putreg(m,a,v);}
static void sync(void*p){(void)p;}
static void put(unsigned char*p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(n>>(i*8));}
static unsigned packet(unsigned char*b,unsigned char key[32],unsigned extent){
 memset(b,0,RT_CAPACITY);unsigned n=32+extent;b[0]='B';b[1]='6';b[2]='U';b[3]='P';b[4]=1;b[8]=(unsigned char)n;b[9]=(unsigned char)(n>>8);unsigned char*e=b+12;put(e,0x31423646);e[4]=1;put(e+8,1);put(e+12,1);put(e+16,0x08020200);put(e+20,extent);put(e+32,0x20020000);put(e+36,0x08020401);unsigned char seed[32]={0},secret[64];crypto_ed25519_key_pair(secret,key,seed);crypto_ed25519_sign(e+n,secret,e,n);return 12+n+64;
}
int main(void){struct rt_frame f;unsigned char wire[RT_CAPACITY];struct fb_policy p={0};struct bs_layouts l={.core={{{0x08020200,0x08022000,0x20000000,0x20020000},{0x08040200,0x08042000,0x20000000,0x20020000}},{{0x08060200,0x08062000,0x30000000,0x30020000},{0x08080200,0x08082000,0x30000000,0x30020000}}}};p.present=2;p.floor[0]=p.floor[1]=p.generation_floor=1;struct fb_image out,before;memset(&before,0xA5,sizeof before);
 for(unsigned extent=514;extent<=2048;extent+=1534){unsigned n=packet(wire,p.keys[0],extent);rt_reset(&f);out=before;for(unsigned i=0;i<n;++i){C(rt_byte(&f,wire[i])==(i+1==n?RT_READY:RT_MORE));if(i+1<n)C(!rt_admit(&f,&p,&l,&out)&&!memcmp(&out,&before,sizeof out));}C(rt_admit(&f,&p,&l,&out)&&out.extent==extent);C(rt_byte(&f,0)==RT_BAD&&!rt_admit(&f,&p,&l,&out));}
 unsigned n=packet(wire,p.keys[0],576);for(unsigned cut=0;cut<n;++cut){rt_reset(&f);for(unsigned i=0;i<cut;++i)C(rt_byte(&f,wire[i])==RT_MORE);out=before;C(!rt_admit(&f,&p,&l,&out)&&!memcmp(&out,&before,sizeof out));rt_reset(&f);C(!f.used&&!f.state&&f.need==12);}
 for(unsigned pos=0;pos<n;++pos){rt_reset(&f);for(unsigned i=0;i<n;++i){enum rt_result z=rt_byte(&f,(unsigned char)(wire[i]^(i==pos?0x80:0)));if(z==RT_BAD)break;}out=before;C(!rt_admit(&f,&p,&l,&out)&&!memcmp(&out,&before,sizeof out));}
 rt_reset(&f);for(unsigned i=0;i<n;++i)(void)rt_byte(&f,wire[i]);p.revoked=2;out=before;C(!rt_admit(&f,&p,&l,&out));p.revoked=0;
 struct f4_m m={0};struct fh_io io={&m,rd,wr,sync};m.ack=0x600000;C(rt_uart_init(io,3)==1&&rd(&m,U+12)==556&&rd(&m,U)==13);C((rd(&m,0x58020C24)&255)==0x77);C((rd(&m,0x58020C00)&0xF0000)==0xA0000);C(rt_uart_init(io,3)==-1);
 m=(struct f4_m){0};C(rt_uart_init(io,3)==0&&m.reads==3&&rd(&m,U)==0);m=(struct f4_m){0};putreg(&m,0x58024454,3);C(rt_uart_init(io,3)==-1&&!m.reads);
 m=(struct f4_m){0};unsigned char byte=0xA5;uint32_t status=0;C(rt_uart_rx(io,&byte,&status)==0&&byte==0xA5);for(unsigned bit=0;bit<4;++bit){m.ack=32u|(1u<<bit);m.dr_reads=0;byte=0xA5;C(rt_uart_rx(io,&byte,&status)==-1&&byte==0xA5&&m.clear==(1u<<bit)&&m.flush==8&&!m.dr_reads);rt_reset(&f);C(rt_uart_step(io,&f,&status)==-1&&f.state==2);}
 m.ack=32;m.byte=0x42;C(rt_uart_rx(io,&byte,&status)==1&&byte==0x42);rt_reset(&f);C(rt_uart_step(io,&f,&status)==0&&f.used==1);m.ack=0;C(rt_uart_step(io,&f,&status)==1&&f.used==1);
 m=(struct f4_m){0};C(rt_uart_tx(io,0xA5,4)==0&&!m.tx&&m.reads==4);m=(struct f4_m){0};m.ack=128;C(rt_uart_tx(io,0xA5,4)==0&&m.tx==1&&m.reads==5);m=(struct f4_m){0};m.ack=128;m.tc=1;C(rt_uart_tx(io,0x5A,4)==1&&m.tx==1&&m.byte==0x5A);puts("RT_PASS:all truncations,all one-byte corruptions,min/max extent,revocation,UART errors,TC distinct from TXE");return 0;}
