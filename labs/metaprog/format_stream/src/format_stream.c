#include "format_stream.h"
MfpArg mfp_number(uint32_t v){MfpArg a={MFP_NUMBER,v,{NULL,0}};return a;}
MfpArg mfp_bytes(MfpSpan v){MfpArg a={MFP_BYTES,0,v};return a;}
int mfp_parse(MfpSpan s,MfpProgram *p){(void)s;(void)p;return MFP_INVALID;}
int mfp_prepare(const MfpProgram *p,const MfpArg *a,size_t n,MfpMessage *m){(void)p;(void)a;(void)n;(void)m;return MFP_INVALID;}
int mfp_render(const MfpProgram *p,const MfpArg *a,size_t n,MPSink *s){(void)p;(void)a;(void)n;(void)s;return MFP_INVALID;}
int mfp_start(MfpStream *s,const MfpMessage *m){(void)s;(void)m;return MFP_INVALID;}
int mfp_step(MfpStream *s,MfpWriter w,void *c){(void)s;(void)w;(void)c;return MFP_INVALID;}
int mfp_cancel(MfpStream *s){(void)s;return MFP_INVALID;}
