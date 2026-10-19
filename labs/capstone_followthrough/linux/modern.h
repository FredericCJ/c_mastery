#ifndef CAPSTONE_MODERN_H
#define CAPSTONE_MODERN_H
#include "service.h"
#include "ownership.h"
#include "errors.h"
typedef struct { CsSink sink;void *context; } CsResultSink;
er_progress cs_result_step(const unsigned char *,size_t,size_t,CsResultSink *);
typedef struct { OwShared rights;CsWire wire;unsigned generation,live,destructions; } CsSlab;
typedef struct { CsSlab slot[4]; } CsSlabs;
typedef struct { unsigned slot,generation,consumer; } CsSlabToken;
void cs_slabs_init(CsSlabs *);
CsStatus cs_slabs_share(CsSlabs *,const CPMessage *,CsSlabToken[2]);
CsStatus cs_slabs_view(CsSlabs *,CsSlabToken,const unsigned char **,size_t *);
CsStatus cs_slabs_drop(CsSlabs *,CsSlabToken);
typedef struct { CPMessage messages[4];unsigned epoch,count,effects; } CsReceipts;
CsStatus cs_receipt(CsReceipts *,unsigned,const CPMessage *);
typedef struct CsResources CsResources;
typedef struct { CsResources *owner;unsigned id; } CsResourceEntry;
struct CsResources {
    OwScope scope;CsResourceEntry entries[5];void *candidate,*queue;
    int input,output,epoll;unsigned acquired,release_ids[5],released,borrows;
    int modern,published,closing,cleanup_error;
};
int cs_resources_open(CsResources *,int,int,int,unsigned,int);
int cs_resources_close(CsResources *);
int cs_resources_borrow(CsResources *);
int cs_resources_ack(CsResources *);
#endif
