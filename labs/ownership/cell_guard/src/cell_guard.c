#include "cell_guard.h"
int ocg_init(struct ocg_cell *c,int32_t v){(void)c;(void)v;return 1;}
enum ow_status ocg_acquire(struct ocg_cell *c,int w,struct ocg_token *t){(void)c;(void)w;(void)t;return OW_NOT_IMPLEMENTED;}
enum ow_status ocg_release(struct ocg_cell *c,struct ocg_token t){(void)c;(void)t;return OW_NOT_IMPLEMENTED;}
enum ow_status ocg_read(struct ocg_cell *c,struct ocg_token t,int32_t *o){(void)c;(void)t;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status ocg_write(struct ocg_cell *c,struct ocg_token t,int32_t v){(void)c;(void)t;(void)v;return OW_NOT_IMPLEMENTED;}
enum ow_status ocg_replace(struct ocg_cell *c,int32_t v,int32_t *o){(void)c;(void)v;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status ocg_update(struct ocg_cell *c,ocg_update_fn f,void *p,int32_t *o){(void)c;(void)f;(void)p;(void)o;return OW_NOT_IMPLEMENTED;}
enum ow_status ocg_close(struct ocg_cell *c){(void)c;return OW_NOT_IMPLEMENTED;}
enum ow_status ocg_destroy(struct ocg_cell *c){(void)c;return OW_NOT_IMPLEMENTED;}
