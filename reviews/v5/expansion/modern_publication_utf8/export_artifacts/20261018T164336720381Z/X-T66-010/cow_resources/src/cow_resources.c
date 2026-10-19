#include "cow_resources.h"
OwStatus ocr_init(OcrStore *s,OcrMemory m,OcrOps o,const OcrResource seed[3]){(void)s;(void)m;(void)o;(void)seed;return OW_NOT_IMPLEMENTED;}
OwStatus ocr_snapshot(OcrStore *s,OcrSnapshot *o){(void)s;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus ocr_read(OcrSnapshot s,size_t r,size_t e,int32_t *o){(void)s;(void)r;(void)e;(void)o;return OW_NOT_IMPLEMENTED;}
OwStatus ocr_release(OcrSnapshot s){(void)s;return OW_NOT_IMPLEMENTED;}
OwStatus ocr_write(OcrStore *s,size_t r,size_t e,int32_t v){(void)s;(void)r;(void)e;(void)v;return OW_NOT_IMPLEMENTED;}
OwStatus ocr_close(OcrStore *s){(void)s;return OW_NOT_IMPLEMENTED;}
OwStatus ocr_destroy(OcrStore *s){(void)s;return OW_NOT_IMPLEMENTED;}
