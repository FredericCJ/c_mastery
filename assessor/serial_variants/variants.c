#include "variants.h"
#ifndef SV_MUTANT
#define SV_MUTANT 0
#endif
int sv_baud(uint32_t clock,uint32_t baud,uint32_t ppm,struct sv_baud *out){
 if(!out||!clock||clock>180000000u||!baud||baud>12000000u||ppm>1000000u)return SV_INVALID;
 uint32_t top=4u*clock,q=top/baud,r=top%baud;
 if(r>=baud-r)q++;
 if(q<64u||q>4194303u)return SV_INVALID;
 uint64_t desired=(uint64_t)baud*q,diff=desired>top?desired-top:top-desired;
 if(diff*1000000u>(uint64_t)ppm*desired)return SV_INVALID;
 struct sv_baud v={q/64u,q%64u,q};
 if(SV_MUTANT==1)v.fraction=q%16u;
 *out=v;return SV_OK;
}
int sv_word(unsigned m,unsigned parity,unsigned stop,unsigned p,unsigned h,unsigned l,uint16_t value,struct sv_word *out){
 if(!out||m>1||parity>2||(stop!=1&&stop!=2)||p>1||h>1||l>1)return SV_INVALID;
 unsigned bits=8u+m,payload=bits-(parity!=0);
 if(value>=(1u<<payload))return SV_INVALID;
 struct sv_word v={0};v.payload=payload;v.length=1u+bits+stop;unsigned bitxor=0;
 for(unsigned i=0;i<payload;i++){v.wire[1u+i]=(uint8_t)((value>>i)&1u);bitxor^=v.wire[1u+i];}
 if(parity)v.wire[bits]=(uint8_t)(bitxor^(parity==2));
 for(unsigned i=1u+bits;i<v.length;i++)v.wire[i]=1;
 v.edges=2u*(bits-!l);
 if(SV_MUTANT==2&&m&&parity)v.edges+=2;
 if(v.edges>18)return SV_IO;
 for(unsigned i=0;i<v.edges;i++){v.clock[i]=(uint8_t)(p^((i+1u)&1u));v.sample[i]=(uint8_t)((i&1u)==h);}
 *out=v;return SV_OK;
}
int sv_capture(unsigned p,unsigned h,uint32_t period,const struct sv_edge *edges,size_t n,struct sv_capture *out){
 if(!out||!edges||p>1||h>1||!period||period>160000000u||n<2||n>18||(n&1u))return SV_INVALID;
 for(size_t i=0;i<n;i++)if(edges[i].level>1||edges[i].sample>1)return SV_INVALID;
 uint32_t minimum=period/16u+(period%16u!=0);struct sv_capture v={0};
 for(size_t i=0;i<n;i++){
  unsigned flags=(edges[i].level!=(p^((i+1u)&1u)))|((edges[i].sample!=((i&1u)==h))?2u:0u);
  if(edges[i].sample){if(edges[i].setup_ns<minimum)flags|=4;if(edges[i].hold_ns<minimum&&SV_MUTANT!=3)flags|=8;}
  if(flags){v.index=(unsigned)i;v.defects=flags;*out=v;return SV_IO;}
 }
 *out=v;return SV_OK;
}
int sv_receiver(int32_t a,int32_t b,unsigned enabled,unsigned open,unsigned characterized,int low,int high,unsigned prior,unsigned *out){
 if(!out||enabled>1||open>1||characterized>1||prior>SV_UNKNOWN||a< -15000||a>15000||b< -15000||b>15000)return SV_INVALID;
 int64_t delta=(int64_t)a-b;if(delta< -15000||delta>15000)return SV_INVALID;
 if(characterized&&(low< -200||high> -20||high-low<30))return SV_INVALID;
 unsigned value;
 if(!enabled)value=SV_HIGHZ;
 else if(open)value=SV_HIGH;
 else if(characterized){value=prior;if(delta<low)value=SV_LOW;else if(delta>high)value=SV_HIGH;}
 else value=delta< -200?SV_LOW:delta> -20?SV_HIGH:SV_UNKNOWN;
 if(SV_MUTANT==4&&enabled&&!open&&delta<0)value=SV_LOW;
 *out=value;return SV_OK;
}
int sv_stream_reset(struct sv_stream *s,uint32_t g){
 if(!s||!g)return SV_INVALID;
 if(g<=s->generation)return SV_STALE;
 struct cn_stream empty={0};s->inner=empty;s->generation=g;return SV_OK;
}
int sv_stream_feed(struct sv_stream *s,uint32_t g,const uint8_t *p,size_t n,size_t *used,struct cn_decoded *out){
 if(!s||!used||!out||(!p&&n))return SV_INVALID;
 if((!g||g!=s->generation)&&SV_MUTANT!=5)return SV_STALE;
 int r=cn_feed(&s->inner,p,n,used,out);
 return r==CN_MORE?SV_MORE:r==CN_FRAME?SV_FRAME:r==CN_INVALID?SV_INVALID:r==CN_BUSY?SV_STALE:SV_IO;
}
int sv_policy(unsigned ext,unsigned tx,unsigned srr,unsigned r1,unsigned r0,unsigned eof,struct sv_policy *out){
 if(!out||ext>1||tx>1||srr>1||r1>1||r0>1||eof>1||(!ext&&(!srr||r1)))return SV_INVALID;
 struct sv_policy v={SV_ACCEPT,0};
 if(tx){if((ext&&!srr)||r1||r0||!eof)v.action=SV_FORM_ERROR;}
 else if((ext&&r1)||(!ext&&r0))v.action=SV_PROTOCOL_EXCEPTION;
 else if(!eof){v.overload=1;if(SV_MUTANT==6)v.action=SV_FORM_ERROR;}
 *out=v;return SV_OK;
}
static int key(const struct sd_can *p,uint32_t *out){
 if(!p||p->extended>1||p->remote>1||p->identifier>(p->extended?0x1fffffffu:0x7ffu))return SV_INVALID;
 *out=p->extended?(p->identifier<<3)|4u|(p->remote<<1):(p->identifier<<21)|(p->remote<<1);return SV_OK;
}
int sv_bank(unsigned list,const struct sd_can *a,const struct sd_can *b,uint32_t mask,struct sv_bank *out){
 if(!out||list>1)return SV_INVALID;
 struct sv_bank v={0};v.list=list;
 if(key(a,&v.first))return SV_INVALID;
 if(list){if(key(b,&v.second))return SV_INVALID;}
 else {if(mask>(a->extended?0x1fffffffu:0x7ffu))return SV_INVALID;v.second=(a->extended?mask<<3:mask<<21)|6u;if(SV_MUTANT==7)v.second&=~6u;}
 *out=v;return SV_OK;
}
int sv_filter(const struct sv_bank *bank,const struct sd_can *frame,unsigned *out){
 if(!bank||!out||bank->list>1)return SV_INVALID;
 uint32_t bits;if(key(frame,&bits))return SV_INVALID;
 *out=bank->list?(bits==bank->first||bits==bank->second):((bits&bank->second)==(bank->first&bank->second));return SV_OK;
}
