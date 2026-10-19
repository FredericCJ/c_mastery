#include "cell_guard.h"
#include <errno.h>
#ifndef OCG_MUTATION
#define OCG_MUTATION 0
#endif
static int enter(OcgCell *c){return c&&c->ready&&pthread_mutex_lock(&c->mutex)==0;}
static OwStatus leave(OcgCell *c,OwStatus s){(void)pthread_mutex_unlock(&c->mutex);return s;}
int ocg_init(OcgCell *c,int32_t value){if(!c)return EINVAL;int e=pthread_mutex_init(&c->mutex,NULL);if(e)return e;c->cell=(OwCell){0};c->cell.value=value;c->cell.borrows.extent=1;c->closed=c->updating=0;c->ready=1;return 0;}
OwStatus ocg_acquire(OcgCell *c,int writer,OcgToken *out){if(!out||!enter(c))return OW_INVALID;if(c->closed)return leave(c,OW_DEAD);OwToken t;OwStatus s=ow_borrow(&c->cell.borrows,0,1,writer,&t);if(s==OW_OK)*out=(OcgToken){c,t};return leave(c,s);}
OwStatus ocg_release(OcgCell *c,OcgToken t){if(!enter(c))return OW_INVALID;OwStatus s=t.owner==c||OCG_MUTATION==4?ow_release(&c->cell.borrows,t.token):OW_STALE;return leave(c,s);}
OwStatus ocg_read(OcgCell *c,OcgToken t,int32_t *out){if(!out||!enter(c))return OW_INVALID;OwStatus s=t.owner==c||OCG_MUTATION==4?ow_cell_read(&c->cell,t.token,out):OW_STALE;return leave(c,s);}
OwStatus ocg_write(OcgCell *c,OcgToken t,int32_t value){if(!enter(c))return OW_INVALID;OwStatus s=t.owner==c||OCG_MUTATION==4?ow_cell_write(&c->cell,t.token,value):OW_STALE;return leave(c,s);}
OwStatus ocg_replace(OcgCell *c,int32_t value,int32_t *old){if(!old||!enter(c))return OW_INVALID;OwStatus s=c->closed?OW_DEAD:ow_cell_replace(&c->cell,value,old);return leave(c,s);}
OwStatus ocg_update(OcgCell *c,OcgUpdate fn,void *context,int32_t *old){
 if(!fn||!old||!enter(c))return OW_INVALID;
 if(c->closed)return leave(c,OW_DEAD);
 if(c->updating)return leave(c,OW_BUSY);
 OwToken guard;OwStatus s=ow_borrow(&c->cell.borrows,0,1,1,&guard);if(s!=OW_OK)return leave(c,s);
 int32_t before; s=ow_cell_read(&c->cell,guard,&before);if(s!=OW_OK){(void)ow_release(&c->cell.borrows,guard);return leave(c,s);}
 c->updating=1;if(OCG_MUTATION==1)(void)ow_release(&c->cell.borrows,guard);
 (void)pthread_mutex_unlock(&c->mutex);int32_t candidate=before;int accepted=fn(context,before,&candidate)==0;
 /* Valid normal mutex and no owner death/cancellation/nonlocal exit are premises. */
 (void)pthread_mutex_lock(&c->mutex);
 if(OCG_MUTATION==1)(void)ow_borrow(&c->cell.borrows,0,1,1,&guard);
 if(c->closed&&OCG_MUTATION!=3)s=OW_DEAD;
 else if(!accepted&&OCG_MUTATION!=2)s=OW_INVALID;
 else{s=ow_cell_write(&c->cell,guard,candidate);if(s==OW_OK)*old=before;}
 (void)ow_release(&c->cell.borrows,guard);c->updating=0;return leave(c,s);
}
OwStatus ocg_close(OcgCell *c){if(!enter(c))return OW_INVALID;c->closed=1;return leave(c,OW_OK);}
OwStatus ocg_destroy(OcgCell *c){if(!enter(c))return OW_INVALID;if(c->updating)return leave(c,OW_BUSY);for(unsigned i=0;i<4;++i)if(c->cell.borrows.loan[i].active)return leave(c,OW_BUSY);c->closed=1;(void)pthread_mutex_unlock(&c->mutex);if(pthread_mutex_destroy(&c->mutex)!=0)return OW_BUSY;c->ready=0;return OW_OK;}
