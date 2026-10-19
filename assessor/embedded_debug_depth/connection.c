#include "connection.h"
#ifndef DC_MUTANT
#define DC_MUTANT 0
#endif
static int shape(const DCManifest*m){
 if(!m||(m->known&~1023u))return 0;
 const unsigned values[]={m->board,m->probe,m->firmware,m->server,m->config,m->target,m->reset,m->power,m->flash_driver,m->transport};
 for(unsigned i=0;i<10;++i)if(((m->known>>i)&1)!=(values[i]!=0))return 0;
 if(!(m->known&DC_TRANSPORT)&&m->latency_ns)return 0;
 return 1;
}
enum dc_status dc_init(DCSession*s,const DCManifest*m){if(!s||!shape(m))return DC_INVALID;*s=(DCSession){.manifest=*m};return DC_OK;}
enum dc_status dc_identify(DCSession*s,unsigned t){
 if(!s||!t)return DC_INVALID;
 if(s->phase!=DC_DISCONNECTED)return DC_ORDER;
 if((s->manifest.known&255)!=255)return DC_UNKNOWN;
 if(DC_MUTANT!=1&&t!=s->manifest.target)return DC_MISMATCH;
 s->observed_target=t;s->phase=DC_IDENTIFIED;return DC_OK;
}
enum dc_status dc_halt(DCSession*s,unsigned v){if(!s||v>1)return DC_INVALID;if(s->phase!=DC_IDENTIFIED)return DC_ORDER;if(!v)return DC_UNKNOWN;s->phase=DC_HALTED;return DC_OK;}
enum dc_status dc_reset(DCSession*s,unsigned v){if(!s||v>1)return DC_INVALID;if(s->phase!=DC_HALTED)return DC_ORDER;if(!v)return DC_UNKNOWN;s->phase=DC_RESET_TESTED;return DC_OK;}
enum dc_status dc_program_authorized(const DCSession*s){if(!s)return DC_INVALID;if(s->phase!=DC_RESET_TESTED)return DC_ORDER;if(DC_MUTANT!=2&&!(s->manifest.known&DC_FLASH_DRIVER))return DC_UNKNOWN;return DC_OK;}
enum dc_status dc_reconfigure(DCSession*s,const DCManifest*m){
 if(!s||!shape(m))return DC_INVALID;
 const DCManifest*a=&s->manifest;
 unsigned same=a->known==m->known&&a->board==m->board&&a->probe==m->probe&&a->firmware==m->firmware&&a->server==m->server&&a->config==m->config&&a->target==m->target&&a->reset==m->reset&&a->power==m->power&&a->flash_driver==m->flash_driver&&a->transport==m->transport&&a->latency_ns==m->latency_ns;
 if(same)return DC_OK;
 s->manifest=*m;if(DC_MUTANT!=3){s->phase=DC_DISCONNECTED;s->observed_target=0;}return DC_OK;
}
enum dc_status dc_classify(unsigned c,unsigned p,unsigned r,unsigned*out){if(!out||c>1||p>1||r>1||(!c&&p)||(!p&&r))return DC_INVALID;*out=!c?DC_CONNECTION:!p?DC_FLASH:!r?DC_RESET_APPLICATION:DC_SUCCESS;return DC_OK;}
enum dc_status dc_vector(uint32_t b,uint32_t n,uint32_t v,unsigned*out){if(!out||!n||n>UINT32_MAX-b)return DC_INVALID;*out=(v&1)&&((v&~UINT32_C(1))>=b)&&((v&~UINT32_C(1))-b<n)&&n-((v&~UINT32_C(1))-b)>=2;return DC_OK;}
