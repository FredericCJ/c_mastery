#include "trial_intent.h"
#include "monocypher-ed25519.h"
#include <stdio.h>
#include <string.h>
#define C(x) do{if(!(x)){fprintf(stderr,"FT_CHECK:%d:%s\n",__LINE__,#x);return __LINE__;}}while(0)
struct f4_model {uint32_t mem[262144],cr,sr,base,words[8],address[8];unsigned key,busy,erase,count,fresh,violations,cut,step,dead,erasecut,error,corrupt;};
static struct f4_model m;static struct fb_policy policy;static struct bs_layouts layout={.core={{{0x08020200,0x08022000,0x20000000,0x20020000},{0x08040200,0x08042000,0x20000000,0x20020000}},{{0x08060200,0x08062000,0x30000000,0x30020000},{0x08080200,0x08082000,0x30000000,0x30020000}}}};
static uint32_t rd(void*c,uint32_t a){struct f4_model*s=c;if(a==0x5200200C)return s->cr;if(a==0x52002010)return s->sr;if(a>=0x08000000&&a<0x08100000){if(s->busy)++s->violations;return s->mem[(a-0x08000000)/4];}++s->violations;return 0;}
static int read_slot(void*c,uint32_t a,uint32_t*out){if((a&3)||a<0x08000000||a>=0x08100000)return 0;*out=rd(c,a);return 1;}
static void wr(void*c,uint32_t a,uint32_t n){struct f4_model*s=c;if(s->dead)return;if(a==0x52002004){if(!s->key&&n==0x45670123)s->key=1;else if(s->key==1&&n==0xCDEF89AB){s->cr&=~1u;s->key=0;}else ++s->violations;return;}if(a==0x52002014){s->sr&=~n;return;}if(a==0x5200200C){if(s->busy){++s->violations;return;}s->cr=n;if(n&0x80){s->base=0x08000000+((n>>8)&7)*0x20000;if(s->base!=0x080A0000)++s->violations;s->erase=1;s->busy=1;s->sr=5;}return;}if(a>=0x080A0000&&a<0x080C0000){if(!(s->cr&2)||s->count>=8)++s->violations;else{s->address[s->count]=a;s->words[s->count++]=n;s->busy=1;s->sr=5;}return;}++s->violations;}
static void sync_io(void*c){(void)c;}
static void fresh(void*c,uint32_t a,uint32_t n){struct f4_model*s=c;if(s->busy||a<0x080A0000||a+n>0x080C0000)++s->violations;++s->fresh;if(s->corrupt&&s->fresh==2)s->mem[0xA0000/4+39]^=1;}
static struct fm_io io(void){return(struct fm_io){{&m,rd,wr,sync_io},fresh};}
static void finish(void){if(!m.busy||m.dead)return;if(!m.error){if(m.erase){unsigned start=(m.base-0x08000000)/4;for(unsigned i=0;i<32768;i+=8){for(unsigned j=0;j<8;++j)m.mem[start+i+j]=UINT32_MAX;if(m.erasecut==i/8+1){m.dead=1;return;}}}else for(unsigned i=0;i<m.count;++i){m.mem[(m.address[i]-0x08000000)/4]&=m.words[i];if(++m.step==m.cut){m.dead=1;return;}}}m.busy=m.erase=m.count=0;m.cr&=~0x80u;m.sr=m.error?0x20000u:0x10000u;}
static void put(unsigned char*p,uint32_t n){for(unsigned i=0;i<4;++i)p[i]=(unsigned char)(n>>(8*i));}
static uint32_t get(const unsigned char*p){return (uint32_t)p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
static void make(struct rt_frame*f,unsigned core,unsigned slot,unsigned generation){unsigned char packet[684]={0};packet[0]='B';packet[1]='6';packet[2]='U';packet[3]='P';packet[4]=1;packet[5]=(unsigned char)core;packet[6]=(unsigned char)slot;packet[8]=0x60;packet[9]=2;unsigned char*e=packet+12;put(e,0x31423646);e[4]=1;e[5]=(unsigned char)core;put(e+8,generation);put(e+12,generation);put(e+16,layout.core[core][slot].base);put(e+20,576);put(e+32,layout.core[core][slot].stack_high);put(e+36,layout.core[core][slot].base+513);unsigned char seed[32]={0},secret[64];crypto_ed25519_key_pair(secret,policy.keys[0],seed);crypto_ed25519_sign(e+608,secret,e,608);rt_reset(f);for(unsigned i=0;i<684;++i)(void)rt_byte(f,packet[i]);}
static void store(uint32_t a,const unsigned char*p,unsigned n){for(unsigned i=0;i<n;i+=4)m.mem[(a-0x08000000+i)/4]=get(p+i);}
static void init(const unsigned char*record,const struct rt_frame old[2]){memset(&m,0,sizeof m);m.cr=0x31;for(unsigned i=0;i<262144;++i)m.mem[i]=UINT32_MAX;for(unsigned i=0;i<32768;++i)m.mem[0xA0000/4+i]=0;for(unsigned c=0;c<2;++c){uint32_t base=layout.core[c][0].base;store(base-32,old[c].bytes+12,608);store(base+2048,old[c].bytes+620,64);}store(0x080C0000,record,128);}

static enum fh_result drain(struct ft_writer*w){enum fh_result r=FH_PENDING;for(unsigned i=0;i<1200&&r==FH_PENDING&&!m.dead;++i){finish();if(!m.dead)r=ft_poll(w);}return r;}
static void trial_store(const struct rt_frame newer[2]){for(unsigned c=0;c<2;++c){uint32_t base=layout.core[c][1].base;store(base-32,newer[c].bytes+12,608);store(base+2048,newer[c].bytes+620,64);}}
static uint32_t fail_address;
static int guarded(void*c,uint32_t a,uint32_t*out){if(a==fail_address)return 0;return read_slot(c,a,out);}
int main(void){policy.present=2;policy.floor[0]=policy.floor[1]=policy.generation_floor=1;struct rt_frame prior[2],newer[2];struct fb_image previous[2],candidate[2];
 for(unsigned c=0;c<2;++c){make(&prior[c],c,0,1);C(rt_admit(&prior[c],&policy,&layout,&previous[c]));make(&newer[c],c,1,2);C(rt_admit(&newer[c],&policy,&layout,&candidate[c]));}
 struct fw_peer peer={.generation=1,.health=3,.confirmed=1};unsigned char record[128],intent[128],attempt[32];C(fb_joint_record(previous,&peer,1,record));struct bs_selection old={.sequence=1,.record=0};old.image[0]=previous[0];old.image[1]=previous[1];C(ft_intent(&old,candidate,intent)&&ft_attempt(intent,attempt));
 const uint32_t records[2]={0x080C0000,0x080E0000};struct bs_selection selected,out,before;memset(&before,0xA5,sizeof before);
 for(unsigned cut=0;cut<=40;++cut){init(record,prior);trial_store(newer);m.cut=cut;struct ft_writer w={0};C(ft_begin(&w,io(),&old,candidate)==FH_PENDING);C(ft_poll(&w)==FH_PENDING);enum fh_result z=drain(&w);C(!m.violations);C(!memcmp((unsigned char*)m.mem+0xC0000,record,128));
  /* A reset never reprograms an all-ones-looking attempt word: cold boot
   * ignores the trial sector, including a complete but unacknowledged attempt. */
  m.busy=0;C(bs_select(guarded,&m,&policy,&layout,records,&selected)==1&&selected.sequence==1&&selected.image[0].base==old.image[0].base);
  if(cut)C(m.dead);else {C(z==FH_DONE&&w.state==4&&m.step==40&&m.fresh==2);C(ft_probe(guarded,&m,&policy,&layout,&old,&out)&&out.sequence==2&&out.image[1].generation==2);}
 }
 const unsigned erasecuts[]={1,2048,4096};for(unsigned i=0;i<3;++i){init(record,prior);trial_store(newer);m.erasecut=erasecuts[i];struct ft_writer w={0};C(ft_begin(&w,io(),&old,candidate)==FH_PENDING);(void)drain(&w);C(m.dead);m.busy=0;C(bs_select(guarded,&m,&policy,&layout,records,&selected)==1&&selected.sequence==1);}
 init(record,prior);trial_store(newer);store(0x080A0000,intent,128);store(0x080A0080,attempt,32);
 C(ft_probe(guarded,&m,&policy,&layout,&old,&out)&&out.sequence==2);
 for(unsigned word=0;word<40;++word){fail_address=0x080A0000+word*4;out=before;C(!ft_probe(guarded,&m,&policy,&layout,&old,&out)&&!memcmp(&out,&before,sizeof out));}fail_address=0;
 for(unsigned word=0;word<40;++word){m.mem[0xA0000/4+word]^=1;out=before;C(!ft_probe(guarded,&m,&policy,&layout,&old,&out)&&!memcmp(&out,&before,sizeof out));m.mem[0xA0000/4+word]^=1;}
 for(unsigned c=0;c<2;++c)for(unsigned word=0;word<168;++word){fail_address=layout.core[c][1].base-32+(word<152?4*word:2080+4*(word-152));out=before;C(!ft_probe(guarded,&m,&policy,&layout,&old,&out)&&!memcmp(&out,&before,sizeof out));}fail_address=0;
 struct bs_selection moved=old;moved.sequence=2;out=before;C(!ft_probe(guarded,&m,&policy,&layout,&moved,&out)&&!memcmp(&out,&before,sizeof out));
 policy.revoked=2;C(!ft_probe(guarded,&m,&policy,&layout,&old,&out));policy.revoked=0;policy.floor[0]=3;C(!ft_probe(guarded,&m,&policy,&layout,&old,&out));policy.floor[0]=1;
 unsigned char unchanged[128];memset(unchanged,0xA5,128);memcpy(intent,unchanged,128);moved=old;moved.sequence=UINT32_MAX;C(!ft_intent(&moved,candidate,intent)&&!memcmp(intent,unchanged,128));candidate[0].generation=1;C(!ft_intent(&old,candidate,intent)&&!memcmp(intent,unchanged,128));candidate[0].generation=2;
 init(record,prior);trial_store(newer);m.corrupt=1;struct ft_writer w={0};C(ft_begin(&w,io(),&old,candidate)==FH_PENDING);C(drain(&w)==FH_ERROR);
 init(record,prior);trial_store(newer);m.error=1;w=(struct ft_writer){0};C(ft_begin(&w,io(),&old,candidate)==FH_PENDING);C(drain(&w)==FH_ERROR&&!m.violations);
 puts("FT_PASS:40 program-store cuts,3 erase cuts,cold committed fallback,376 guarded-word faults,40 corruptions,parent and attempt admission");return 0;}
