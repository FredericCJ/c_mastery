#include "cow_resources.h"
enum ow_status ocr_init(struct ocr_store *s,struct ocr_memory m,struct ocr_ops o,const struct ocr_resource seed[3]){(void)s;(void)m;(void)o;(void)seed;return OW_NOT_IMPLEMENTED;}
enum ow_status ocr_snapshot(struct ocr_store *s,struct ocr_snapshot *o){(void)s;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status ocr_read(struct ocr_snapshot s,size_t r,size_t e,int32_t *o){(void)s;(void)r;(void)e;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status ocr_release(struct ocr_snapshot s){(void)s;return OW_NOT_IMPLEMENTED;}
enum ow_status ocr_write(struct ocr_store *s,size_t r,size_t e,int32_t v){(void)s;(void)r;(void)e;(void)v;return OW_NOT_IMPLEMENTED;}
enum ow_status ocr_close(struct ocr_store *s){(void)s;return OW_NOT_IMPLEMENTED;}
enum ow_status ocr_destroy(struct ocr_store *s){(void)s;return OW_NOT_IMPLEMENTED;}
