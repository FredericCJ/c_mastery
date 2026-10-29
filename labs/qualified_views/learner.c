#include "views.h"
enum fv_status fv_reset(struct fv_record *p){(void)p;return FV_UNFINISHED;}
struct fv_view fv_initialize(struct fv_record *p){(void)p;return (struct fv_view){0};}
enum fv_status fv_update(struct fv_record *p,size_t i,int v){(void)p;(void)i;(void)v;return FV_UNFINISHED;}
struct fv_view fv_view(const struct fv_record *p){(void)p;return (struct fv_view){0};}
enum fv_status fv_copy(struct fv_destination d,struct fv_view s){(void)d;(void)s;return FV_UNFINISHED;}
enum fv_status fv_move(int *b,size_t e,size_t d,size_t s,size_t n){(void)b;(void)e;(void)d;(void)s;(void)n;return FV_UNFINISHED;}
struct fv_owner *fv_owner(void){return NULL;}
struct fv_view fv_owner_view(const struct fv_owner *p){(void)p;return (struct fv_view){0};}
int *fv_owner_mutable(struct fv_owner *p){(void)p;return NULL;}
unsigned fv_device_twice(volatile unsigned *p){(void)p;return 1;}
