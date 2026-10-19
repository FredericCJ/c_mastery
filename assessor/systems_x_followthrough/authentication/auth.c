#include "auth.h"
#ifndef SX_AUTH_MUTANT
#define SX_AUTH_MUTANT 0
#endif
int sx_auth_encode(uint64_t id,const B5Redo*r,unsigned char m[32]){static const unsigned char domain[8]={'S','X','R','E','D','O','1',0};uint32_t fields[4];if(!id||!r||!m||r->commit>1)return B5_INVALID;fields[0]=r->generation;fields[1]=r->value;fields[2]=r->checksum;fields[3]=r->commit;for(unsigned i=0;i<8;i++){m[i]=domain[i];m[8+i]=(unsigned char)(SX_AUTH_MUTANT==2?0:id>>(8*i));}for(unsigned j=0;j<4;j++)for(unsigned i=0;i<4;i++)m[16+4*j+i]=(unsigned char)(fields[j]>>(8*i));return B5_OK;}
int sx_auth_recover(uint64_t id,uint32_t checkpoint,uint32_t base,const SxAuthenticated*r,size_t n,SxMac mac,void*context,uint32_t*out){B5Redo raw[8];size_t prefix=0;if(!id||!out||(!r&&n)||n>8||!mac)return B5_INVALID;for(size_t i=0;i<n;i++){unsigned char m[32],tag[32];unsigned difference=0;if(r[i].record.commit>1)return B5_INVALID;raw[i]=r[i].record;prefix=i+1;if(!raw[i].commit)break;if(sx_auth_encode(id,&raw[i],m)!=B5_OK)return B5_INVALID;if(mac(context,m,32,tag)!=B5_OK)return B5_CONFLICT;for(unsigned j=0;j<32;j++)difference|=tag[j]^r[i].tag[j];if(difference&&SX_AUTH_MUTANT!=1)return B5_CONFLICT;}
 return b5_recover(checkpoint,base,raw,prefix,out);
}
