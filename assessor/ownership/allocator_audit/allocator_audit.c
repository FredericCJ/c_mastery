#include "allocator_audit.h"
#include <limits.h>
#ifndef OAA_MUTATION
#define OAA_MUTATION 0
#endif
static void increment(OaaAudit *a,size_t *value){if(*value==SIZE_MAX)a->exhausted=1;else ++*value;}
static void failed(OaaAudit *a,OwStatus status){
 if(!a->report||a->reporting)return;
 OaaFailure event={status,a->attempts,a->live,a->live_cells};a->reporting=1;increment(a,&a->reports);a->report(a->report_context,event);a->reporting=0;
}
static int attempt(OaaAudit *a){
 if(a->attempts==SIZE_MAX){a->exhausted=1;return 0;}
 if(!(OAA_MUTATION==4&&a->reporting))++a->attempts;
 if(a->reporting)increment(a,&a->nested_attempts);
 return a->attempts!=a->fail_at;
}
OwStatus oaa_init(OaaAudit *a,OaaUpstream upstream){if(!a||!upstream.base.take||!upstream.base.release||!upstream.resize)return OW_INVALID;*a=(OaaAudit){0};a->upstream=upstream;return OW_OK;}
OwStatus oaa_set_report(OaaAudit *a,OaaReporter reporter,void *context){if(!a)return OW_INVALID;if(a->busy||a->reporting)return OW_BUSY;a->report=reporter;a->report_context=context;return OW_OK;}
static int32_t *take(void *context,size_t count){
 OaaAudit *a=context;if(a->busy)return NULL;
 if(!attempt(a)){failed(a,OW_NOMEM);return NULL;}
 if(!count||count>16){failed(a,OW_INVALID);return NULL;}
 unsigned slot=0;while(slot<8&&a->blocks[slot].data)++slot;if(slot==8){failed(a,OW_FULL);return NULL;}
 a->busy=1;increment(a,&a->upstream_calls);int32_t *p=a->upstream.base.take(a->upstream.base.context,count);a->busy=0;
 if(!p){failed(a,OW_NOMEM);return NULL;}
 for(size_t i=0;i<count;++i)p[i]=INT32_MAX;
 a->blocks[slot]=(OaaBlock){p,count};++a->live;a->live_cells+=count;if(a->live_cells>a->peak_cells)a->peak_cells=a->live_cells;increment(a,&a->successes);return p;
}
static void release(void *context,int32_t *p){
 OaaAudit *a=context;if(a->busy)return;
 unsigned slot=0;while(slot<8&&(a->blocks[slot].data!=p||!p))++slot;
 if(slot==8){increment(a,&a->invalid_releases);return;}
 size_t count=a->blocks[slot].count;if(OAA_MUTATION!=3)for(size_t i=0;i<count;++i)p[i]=INT32_MIN;
 a->blocks[slot]=(OaaBlock){0};--a->live;a->live_cells-=count;increment(a,&a->releases);
 a->busy=1;a->upstream.base.release(a->upstream.base.context,p);a->busy=0;
}
V4Allocator oaa_allocator(OaaAudit *a){return (V4Allocator){a,take,release};}
OwStatus oaa_resize(OaaAudit *a,int32_t *old,size_t count,int32_t **out){
 if(!a||!out)return OW_INVALID;
 if(a->busy)return OW_BUSY;
 if(!attempt(a)){failed(a,OW_NOMEM);return OW_NOMEM;}
 unsigned slot=0;while(slot<8&&(a->blocks[slot].data!=old||!old))++slot;
 if(slot==8||!count||count>16){failed(a,OW_INVALID);return OW_INVALID;}
 size_t previous=a->blocks[slot].count;if(count==previous){*out=old;return OW_OK;}
 a->busy=1;increment(a,&a->upstream_calls);int32_t *p=a->upstream.resize(a->upstream.base.context,old,previous,count);a->busy=0;
 if(!p){if(OAA_MUTATION==1)a->live_cells=a->live_cells-previous+count;failed(a,OW_NOMEM);return OW_NOMEM;}
 /* old may now be indeterminate: never compare, dereference or publish it. */
 for(size_t i=previous;i<count;++i)p[i]=INT32_MAX;
 a->blocks[slot]=(OaaBlock){p,count};if(OAA_MUTATION!=2)a->live_cells=a->live_cells-previous+count;
 if(a->live_cells>a->peak_cells)a->peak_cells=a->live_cells;
 increment(a,&a->successes);increment(a,&a->resize_successes);*out=p;return OW_OK;
}
