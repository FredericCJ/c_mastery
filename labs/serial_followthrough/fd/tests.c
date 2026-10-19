#include "fd.h"
void fd_note(const char *);
void fd_values(const char *,unsigned,unsigned,unsigned,unsigned);
#define CHECK(x) do{if(!(x)){fd_note("FD_ASSERT: " #x);return 41;}}while(0)
static int admission(void){
 static const unsigned length[]={0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
 uint8_t bytes[65];for(unsigned i=0;i<65;++i)bytes[i]=(uint8_t)i;
 unsigned cases=0;FdPacket out={0};
 for(unsigned dlc=0;dlc<16;++dlc)for(unsigned brs=0;brs<2;++brs)for(unsigned esi=0;esi<2;++esi){
  FdInput in={dlc,length[dlc],1,brs,esi,0,1};CHECK(fd_admit(&in,bytes,in.length,&out)==FD_OK&&out.metadata.length==length[dlc]&&out.metadata.crc_width==(in.length<=16?17:21));++cases;
 }
 FdInput in={9,12,1,2,0,0,1};out.payload_length=99;
 CHECK(fd_admit(&in,bytes,12,&out)==FD_INVALID&&out.payload_length==99);
 in.brs=0;in.length=9;CHECK(fd_admit(&in,bytes,9,&out)==FD_LENGTH&&out.payload_length==99);
 in=(FdInput){0,0,1,0,0,0,1};CHECK(fd_admit(&in,0,0,&out)==FD_OK);
 in.remote=1;CHECK(fd_admit(&in,0,0,&out)==FD_INVALID);in.remote=0;in.capable=0;CHECK(fd_admit(&in,0,0,&out)==FD_CAPABILITY);
 unsigned exact=0,padded=0;
 for(unsigned n=0;n<=64;++n){
  int status=fd_length(0,bytes,n,1,0,&out);unsigned found=0;for(unsigned i=0;i<16;++i)found|=length[i]==n;
  CHECK(status==(found?FD_OK:FD_LENGTH));exact+=found;
  CHECK(fd_length(1,bytes,n,1,0,&out)==FD_OK&&out.payload_length==n&&out.metadata.length>=n&&out.padding==out.metadata.length-n);
  for(unsigned j=0;j<n;++j)CHECK(out.payload[j]==j);
  for(unsigned j=n;j<64;++j)CHECK(out.payload[j]==0);
  ++padded;
 }
 out.payload_length=99;CHECK(fd_length(1,bytes,65,1,0,&out)==FD_LENGTH&&out.payload_length==99);
 unsigned bit=99;CHECK(fd_esi(0,0,&bit)==FD_OK&&bit==0);CHECK(fd_esi(0,1,&bit)==FD_OK&&bit==1);CHECK(fd_esi(1,0,&bit)==FD_OK&&bit==1);
 fd_values("admission",cases,exact,padded,64);return 0;
}
static int wire(void){
 FdPacket p={0};FdWire a,b;CHECK(fd_length(0,0,0,1,1,&p)==FD_OK);
 CHECK(fd_wire(0x123,&p,1,&a)==FD_OK&&fd_wire(0x123,&p,0,&b)==FD_OK);
 CHECK(a.count==b.count+5&&a.crc_start==b.crc_start&&a.crc!=b.crc);
 CHECK(a.crc==89572&&b.crc==44951); /* separately derived polynomial oracle */
 unsigned different=0;for(unsigned i=0;i<b.count;++i)different+=a.bits[i]!=b.bits[i];CHECK(different>0);
 /* Decode current count/parity independently of the implementation helper. */
 unsigned at=a.crc_start;CHECK(a.bits[at]!=a.bits[at-1]);unsigned sbc=0;
 for(unsigned i=1;i<=4;++i)sbc=(sbc<<1)|a.bits[at+i];
 static const unsigned codes[]={0,3,6,5,12,15,10,9};CHECK(sbc==codes[a.dynamic_stuff%8]);
 fd_values("wire",a.count,b.count,a.crc,b.crc);
 uint8_t bytes[64];uint32_t fingerprint=2166136261u;unsigned largest=0,frames=0;
 for(unsigned dlc=0;dlc<16;++dlc)for(unsigned pattern=0;pattern<4;++pattern){
  for(unsigned i=0;i<64;++i)bytes[i]=(uint8_t)(pattern==0?0:pattern==1?255:pattern==2?(i&1?0x0f:0xf0):i*37+3);
  SDFd metadata;CHECK(sd_fd(dlc,1,1,0,0,1,&metadata)==SD_OK);CHECK(fd_length(0,bytes,metadata.length,1,0,&p)==FD_OK);CHECK(fd_wire(0,&p,1,&a)==FD_OK&&a.count<=709&&a.width==metadata.crc_width);
  for(unsigned j=0;j<a.count;++j)fingerprint=(fingerprint^a.bits[j])*16777619u;
  if(a.count>largest)largest=a.count;
  ++frames;
 }
 fd_values("wire_matrix",frames,largest,fingerprint,709);
 CHECK(frames==64&&largest==681&&fingerprint==UINT32_C(2090412385));
 return 0;
}
static int controller(void){
 uint8_t header[4]={0};size_t used=99;
 CHECK(fd_header(FD_TCAN4550,1,0x8000,16,header,4,&used)==FD_OK&&used==4&&header[0]==0x61&&header[1]==0x80&&header[2]==0&&header[3]==16);
 CHECK(fd_header(FD_MCP2518FD,0,0x400,16,header,4,&used)==FD_OK&&used==2&&header[0]==0x34&&header[1]==0);
 used=99;header[0]=0xa5;CHECK(fd_header(FD_TCAN4550,1,0x87fc,2,header,4,&used)==FD_INVALID&&header[0]==0xa5&&used==99);
 CHECK(fd_header(FD_MCP2518FD,1,0x400,1,header,1,&used)==FD_CAPACITY&&header[0]==0xa5&&used==99);
 FdTransaction t={0};CHECK(fd_transaction(FD_TCAN4550,1,0x8000,64,68,&t)==FD_OK&&t.total_bytes==68&&t.committed_bytes==64);
 CHECK(fd_transaction(FD_MCP2518FD,1,0x400,64,66,&t)==FD_OK&&t.total_bytes==66&&t.committed_bytes==64);
 CHECK(fd_transaction(FD_TCAN4550,1,0x8000,8,10,&t)==FD_OK&&t.committed_bytes==4&&t.ignored_bytes==2);
 CHECK(fd_transaction(FD_MCP2518FD,1,0x400,8,8,&t)==FD_OK&&t.committed_bytes==4&&t.ignored_bytes==2);
 CHECK(fd_transaction(FD_MCP2518FD,0,0xe04,1,3,&t)==FD_OK&&t.committed_bytes==1);
 CHECK(fd_transaction(FD_MCP2518FD,1,0xbfc,8,10,&t)==FD_INVALID);
 CHECK(fd_transaction(FD_TCAN4550,1,0x87fc,8,12,&t)==FD_INVALID);
 CHECK(fd_transaction(FD_TCAN4550,1,0x8001,8,12,&t)==FD_INVALID);
 CHECK(fd_power(FD_TCAN4550,3300,3300,0,0)==FD_INVALID&&fd_power(FD_TCAN4550,12000,3300,0,0)==FD_OK);
 CHECK(fd_power(FD_MCP2518FD,3300,3300,5000,3300)==FD_OK&&fd_power(FD_MCP2518FD,3300,3300,3300,3300)==FD_INVALID);
 fd_values("controller",68,66,4,2);return 0;
}
static int tdc(void){
 uint8_t tx[8]={1,0,1,0,0,1,0,1},rx[85];
 /*10quanta/bit, ordinary sample5, measured loop delay18, offset4.
    Delayed input is compared with the saved transmitted bit, not the current
    output bit two bit periods later. No physical delay measurement is claimed. */
 for(unsigned t=0;t<85;++t)rx[t]=t<18?1:tx[(t-18)/10];
 FdTdc out={0};CHECK(fd_tdc(tx,8,rx,85,10,5,18,4,&out)==FD_OK&&out.checked==7&&out.skipped==1&&out.mismatches==0);
 rx[22]^=1;CHECK(fd_tdc(tx,8,rx,85,10,5,18,4,&out)==FD_OK&&out.mismatches==1&&out.first_reaction==25);
 rx[22]^=1;out.checked=99;CHECK(fd_tdc(tx,8,rx,84,10,5,18,4,&out)==FD_LENGTH&&out.checked==99);
 rx[3]=2;CHECK(fd_tdc(tx,8,rx,85,10,5,18,4,&out)==FD_INVALID&&out.checked==99);
 fd_values("tdc",22,7,1,25);return 0;
}
int fd_test(unsigned n){switch(n){case 0:return admission();case 1:return wire();case 2:return controller();case 3:return tdc();default:return 42;}}
