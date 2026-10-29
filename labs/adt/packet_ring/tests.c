#define _POSIX_C_SOURCE 200809L
#include "packet_ring.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <inttypes.h>
#include <time.h>
#include <errno.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"ADO_CHECK %s:%d: %s\n",__FILE__,__LINE__,#x);return 1;}}while(0)
static struct ad_packet sent(void){static const unsigned char byte=77;return (struct ad_packet){99,&byte,999};}
static int same(struct ad_packet a,struct ad_packet b){return a.kind==b.kind&&a.payload==b.payload&&a.length==b.length;}
static int base(void){
 unsigned char raw[]={1,0,2,7,8};struct ad_packet p=sent(),saved=p;
 CHECK(adp_parse(raw,sizeof raw,&p)==ADP_OK&&p.payload==raw+3&&p.length==2&&p.kind==1);raw[3]=9;CHECK(p.payload[0]==9);
 for(size_t n=0;n<sizeof raw;n++){p=saved;CHECK(adp_parse(raw,n,&p)==(n<3?ADP_HEADER:ADP_LENGTH)&&same(p,saved));}
 unsigned char trailing[]={1,0,1,7,8};p=saved;CHECK(adp_parse(trailing,sizeof trailing,&p)==ADP_LENGTH&&same(p,saved));
 unsigned char ack[]={2,0,1,7};CHECK(adp_parse(ack,sizeof ack,&p)==ADP_ACK&&same(p,saved));ack[0]=3;CHECK(adp_parse(ack,sizeof ack,&p)==ADP_KIND&&same(p,saved));
 CHECK(adp_parse(NULL,0,&p)==ADP_HEADER&&same(p,saved));CHECK(adp_parse(NULL,SIZE_MAX,&p)==ADP_HEADER);
 unsigned char out[8];memset(out,0xa5,sizeof out);size_t used=99;CHECK(adp_build(1,raw,2,out,4,&used)==ADP_FULL&&used==99&&out[0]==0xa5);
 CHECK(adp_build(1,raw,SIZE_MAX,out,sizeof out,&used)==ADP_LENGTH&&used==99&&out[0]==0xa5);
 CHECK(adp_build(2,NULL,0,out,sizeof out,&used)==ADP_OK&&used==3&&adp_parse(out,used,&p)==ADP_OK&&p.length==0);
 unsigned char *large=malloc(65538);CHECK(large);large[0]=1;large[1]=255;large[2]=255;for(size_t i=3;i<65538;i++)large[i]=(unsigned char)i;
 CHECK(adp_parse(large,65538,&p)==ADP_OK&&p.length==65535&&p.payload==large+3);free(large);
 puts("ADP_BASE exact-error-precedence borrowed-cursor checked-size");return 0;
}
static int ring(void){
 unsigned char packet[]={1,0,5,7,8,9,10,11},copy[8],fill[32];memset(fill,88,sizeof fill);
 for(size_t h=0;h<32;h++){
  struct adp_ring r;struct adp_lease v;CHECK(adp_init(&r,h,1)==ADP_OK&&adp_feed(&r,packet,sizeof packet)==ADP_OK);
  CHECK(adp_acquire(&r,sizeof packet,&v)==ADP_OK&&v.length==5&&v.part[0].length+v.part[1].length==5);
  memset(copy,0xa5,sizeof copy);CHECK(adp_copy(&r,v,copy,4)==ADP_FULL&&copy[0]==0xa5);CHECK(adp_copy(&r,v,copy,sizeof copy)==ADP_OK&&!memcmp(copy,packet+3,5));
  CHECK(adp_feed(&r,fill,24)==ADP_OK&&r.count==32);CHECK(adp_copy(&r,v,copy,sizeof copy)==ADP_OK&&!memcmp(copy,packet+3,5));
  CHECK(adp_cancel(&r,v.token)==ADP_OK&&adp_validate(&r,v.token)==ADP_CANCELLED&&r.count==32);
  CHECK(adp_discard(&r,1)==ADP_BUSY&&adp_feed(&r,fill,1)==ADP_FULL);
  CHECK(adp_copy(&r,v,copy,sizeof copy)==ADP_OK&&!memcmp(copy,packet+3,5));CHECK(adp_release(&r,v.token)==ADP_OK&&r.count==24&&r.head==(h+8)%32);
  CHECK(adp_discard(&r,24)==ADP_OK&&r.count==0&&adp_validate(&r,v.token)==ADP_STALE);
 }
 puts("ADP_RING every-origin segmented-zero-copy cancellation-retains-bytes");return 0;
}
static int lifetime(void){
 unsigned char ack[]={2,0,0};struct adp_ring r,other;struct adp_lease a,b;
 CHECK(adp_init(&r,31,1)==ADP_OK&&adp_init(&other,0,1)==ADP_OK);
 CHECK(adp_feed(&r,ack,3)==ADP_OK&&adp_acquire(&r,3,&a)==ADP_OK&&a.part[0].length==0&&a.part[0].data==NULL);
 CHECK(adp_validate(&other,a.token)==ADP_STALE&&adp_release(&r,a.token)==ADP_OK);
 CHECK(adp_feed(&r,ack,3)==ADP_OK&&adp_acquire(&r,3,&b)==ADP_OK&&b.token.generation==2);
 CHECK(adp_release(&r,a.token)==ADP_STALE&&r.active&&adp_validate(&r,b.token)==ADP_OK);
 CHECK(adp_close(&r)==ADP_OK&&adp_validate(&r,b.token)==ADP_CANCELLED&&r.count==3);
 CHECK(adp_feed(&r,ack,3)==ADP_CANCELLED&&adp_acquire(&r,3,&a)==ADP_CANCELLED);
 CHECK(adp_release(&r,b.token)==ADP_OK&&r.count==0&&adp_release(&r,b.token)==ADP_STALE);
 CHECK(adp_init(&r,0,UINT32_MAX)==ADP_OK&&adp_feed(&r,ack,3)==ADP_OK&&adp_acquire(&r,3,&a)==ADP_OK&&a.token.generation==UINT32_MAX);
 CHECK(adp_release(&r,a.token)==ADP_OK&&adp_feed(&r,ack,3)==ADP_OK&&adp_acquire(&r,3,&b)==ADP_RETIRED&&r.count==3);
 puts("ADP_LIFETIME owner-generation release-only-close finite-retirement");return 0;
}
static int pipe_case(const unsigned char *input,size_t n,size_t cap,enum adp_status expected){
 int f[2];CHECK(pipe(f)==0);pid_t child=fork();CHECK(child>=0);
 if(!child){close(f[0]);for(size_t i=0;i<n;i++){ssize_t k;do{k=write(f[1],input+i,1);}while(k<0&&errno==EINTR);if(k!=1)_exit(3);}close(f[1]);_exit(0);}
 close(f[1]);unsigned char storage[32];memset(storage,0xa5,sizeof storage);struct ad_packet p=sent(),before=p;
 enum adp_status got=adp_read_pipe(f[0],storage,cap,&p);close(f[0]);int status;CHECK(waitpid(child,&status,0)==child&&WIFEXITED(status)&&WEXITSTATUS(status)==0);
 CHECK(got==expected);if(got==ADP_OK)CHECK(p.payload==storage+3&&p.kind==input[2]&&p.length==n-5);else CHECK(same(p,before));return 0;
}
static int pipes(void){
 const unsigned char packet[]={0,5,1,0,2,7,8};CHECK(pipe_case(packet,sizeof packet,32,ADP_OK)==0);
 for(size_t n=0;n<sizeof packet;n++)CHECK(pipe_case(packet,n,32,ADP_EOF)==0);
 const unsigned char large[]={0,33};CHECK(pipe_case(large,2,32,ADP_OVERSIZE)==0);
 const unsigned char empty[]={0,0};CHECK(pipe_case(empty,2,32,ADP_HEADER)==0);
 const unsigned char trailing[]={0,5,1,0,1,7,8};CHECK(pipe_case(trailing,sizeof trailing,32,ADP_LENGTH)==0);
 unsigned char wire[8],encoded[5];for(unsigned ver=1;ver<=2;ver++){
  struct ar_record a={ver==1?INT16_MIN:INT32_MIN},b={123};size_t n=ver==1?3:5,used=0;
  CHECK(ar_encode(ver,a,encoded,n)==AR_OK&&adp_build(1,encoded,n,wire,sizeof wire,&used)==ADP_OK);
  struct adp_ring r;struct adp_lease v;CHECK(adp_init(&r,28,1)==ADP_OK&&adp_feed(&r,wire,used)==ADP_OK&&adp_acquire(&r,used,&v)==ADP_OK);
  CHECK(adp_record(&r,v,ver,&b)==ADP_OK&&b.value==a.value);CHECK(adp_release(&r,v.token)==ADP_OK);b.value=123;CHECK(adp_record(&r,v,ver,&b)==ADP_STALE&&b.value==123);
 }
 puts("ADP_PIPE actual-one-byte-pipe-writes truncated-EOF codec-versions");return 0;
}
static int digit(char c){return c>='0'&&c<='9'?c-'0':c>='a'&&c<='f'?c-'a'+10:-1;}
static int oracle(void){
 unsigned origin,cancel;uint32_t generation;size_t requested;char text[100];
 while(scanf("%u %"SCNu32" %u %zu %99s",&origin,&generation,&cancel,&requested,text)==5){
  unsigned char bytes[48],copy[32],fill[32];size_t n=!strcmp(text,"-")?0:strlen(text)/2;
  for(size_t i=0;i<n;i++){int a=digit(text[i*2]),b=digit(text[i*2+1]);if(a<0||b<0)return 2;bytes[i]=(unsigned char)(a*16+b);}
  memset(copy,0xa5,sizeof copy);memset(fill,77,sizeof fill);struct adp_ring r;CHECK(adp_init(&r,origin,generation)==ADP_OK);int feed=adp_feed(&r,bytes,n);struct adp_lease v={0};v.kind=99;v.length=999;
  int acquired=adp_acquire(&r,requested,&v),valid=ADP_INVALID,release=ADP_INVALID,stale=ADP_INVALID;size_t first=0,second=0;unsigned kind=v.kind;size_t length=v.length;
  if(acquired==ADP_OK){first=v.part[0].length;second=v.part[1].length;CHECK(adp_copy(&r,v,copy,sizeof copy)==ADP_OK);if(cancel)CHECK(adp_cancel(&r,v.token)==ADP_OK);valid=adp_validate(&r,v.token);CHECK(adp_feed(&r,fill,32-r.count)==ADP_OK);release=adp_release(&r,v.token);stale=adp_validate(&r,v.token);}
  struct ad_packet parsed=sent(),saved=parsed;int direct=adp_parse(bytes,n,&parsed);int preserved=direct? same(parsed,saved):(parsed.payload==bytes+3&&parsed.length==n-3&&parsed.kind==bytes[0]);
  printf("%d %d %u %zu %zu %zu %d %d %d %zu %zu %"PRIu32" %u %d %d ",feed,acquired,kind,length,first,second,valid,release,stale,r.head,r.count,r.next,r.retired,direct,preserved);for(unsigned i=0;i<32;i++)printf("%02x",copy[i]);putchar('\n');
 }return ferror(stdin)?1:0;
}
static int history(void){
 char operations[12];while(scanf("%11s",operations)==1){struct adp_ring r;struct adp_lease current={0};struct adp_token old={NULL,0};CHECK(adp_init(&r,30,UINT32_MAX-1)==ADP_OK);unsigned char ack[]={2,0,0};
  for(size_t i=0;operations[i];i++){int s=ADP_INVALID;switch(operations[i]){
   case '0':s=adp_feed(&r,ack,3);break;case '1':s=adp_acquire(&r,3,&current);break;case '2':s=adp_cancel(&r,current.token);break;case '3':s=adp_release(&r,current.token);break;
   case '4':old=current.token;s=adp_validate(&r,old);break;case '5':s=adp_release(&r,old);break;case '6':s=adp_discard(&r,3);break;case '7':s=adp_close(&r);break;}
   printf("%s%d,%zu,%zu,%u,%u,%u,%u,%"PRIu32",%"PRIu32,i?";":"",s,r.head,r.count,r.active,r.cancelled,r.closed,r.retired,r.next,r.generation);
  }putchar('\n');
 }return ferror(stdin)?1:0;
}
static int metrics(void){
 unsigned char packet[32]={1,0,29},copy[29];for(unsigned i=3;i<32;i++)packet[i]=(unsigned char)i;
 double borrowing[7],copying[7];const unsigned iterations=10000;uint64_t checksum=0;
 for(unsigned sample=0;sample<7;sample++)for(unsigned mode=0;mode<2;mode++){
  clock_t start=clock();checksum=0;struct adp_ring r;CHECK(adp_init(&r,19,1)==ADP_OK);
  for(unsigned i=0;i<iterations;i++){struct adp_lease v;CHECK(adp_feed(&r,packet,32)==ADP_OK&&adp_acquire(&r,32,&v)==ADP_OK);if(mode){CHECK(adp_copy(&r,v,copy,sizeof copy)==ADP_OK);for(unsigned j=0;j<29;j++)checksum+=copy[j];}else for(unsigned k=0;k<2;k++)for(size_t j=0;j<v.part[k].length;j++)checksum+=v.part[k].data[j];if(i%2)CHECK(adp_cancel(&r,v.token)==ADP_OK);CHECK(adp_release(&r,v.token)==ADP_OK);}
  double seconds=(double)(clock()-start)/CLOCKS_PER_SEC;if(mode)copying[sample]=seconds;else borrowing[sample]=seconds;CHECK(checksum==4930000);
 }
 printf("{\"iterations\":%u,\"checksum\":%"PRIu64",\"payload\":29,\"ring_size\":%zu,\"ring_alignment\":%zu,\"borrow_seconds\":[",iterations,checksum,sizeof(struct adp_ring),_Alignof(struct adp_ring));for(unsigned i=0;i<7;i++)printf("%s%.9f",i?",":"",borrowing[i]);printf("],\"copy_seconds\":[");for(unsigned i=0;i<7;i++)printf("%s%.9f",i?",":"",copying[i]);puts("]}");return 0;
}
int main(int argc,char**argv){if(argc!=2)return 2;
 if(!strcmp(argv[1],"packet_base"))return base();
 if(!strcmp(argv[1],"packet_ring"))return ring();
 if(!strcmp(argv[1],"packet_lifetime"))return lifetime();
 if(!strcmp(argv[1],"packet_pipe"))return pipes();
 if(!strcmp(argv[1],"oracle"))return oracle();
 if(!strcmp(argv[1],"history"))return history();
 if(!strcmp(argv[1],"metrics"))return metrics();
 return 2;
}
