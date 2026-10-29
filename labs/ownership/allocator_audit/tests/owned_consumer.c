#include "allocator_audit.h"
#include "owning_option.h"
#include <stdio.h>
#define CHECK(x) do{if(!(x)){fprintf(stderr,"OW_CHECK OAA_OWNED line%d: %s\n",__LINE__,#x);return 1;}}while(0)
int main(void){
 for(size_t failure=1;failure<=3;++failure){struct ow_allocator upstream={0};struct oaa_audit audit;CHECK(oaa_init(&audit,oaa_ow_upstream(&upstream))==OW_OK);audit.fail_at=failure;struct v4_allocator a=oaa_allocator(&audit);struct eop_owner first={0},second={0};int q=eop_create(&first,a,0,42);CHECK(q==(failure==1?EOP_NOMEM:EOP_OK));if(q==EOP_OK){q=eop_clone(&first,&second);CHECK(q==(failure==2?EOP_NOMEM:EOP_OK));uint32_t id=91;int32_t value=92;CHECK(eop_read(&first,&id,&value)==EOP_OK&&id==0&&value==42);if(q==EOP_NOMEM)CHECK(second.tag==EOP_NONE);else CHECK(second.as.some.cell!=first.as.some.cell);}CHECK(eop_drop(&second)==EOP_OK&&eop_drop(&first)==EOP_OK&&!audit.live&&!upstream.live);CHECK(audit.attempts==(failure==1?1:2)&&audit.releases==(failure<=2?failure-1:2));}
 puts("OAA_OWNED actual-T65-Eop-create-clone zero-ID failure-sweep actual-T66-audit-upstream");return 0;
}
