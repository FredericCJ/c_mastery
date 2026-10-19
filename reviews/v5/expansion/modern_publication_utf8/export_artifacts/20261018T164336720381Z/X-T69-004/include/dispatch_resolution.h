#ifndef CMASTERY_DISPATCH_RESOLUTION_H
#define CMASTERY_DISPATCH_RESOLUTION_H
#include "dispatch.h"
enum {DS_AMBIGUOUS=10};
typedef struct {DSCombine method;DSLoaded *module;} DSMethodLease;
typedef struct {DSMethodLease entries[4][4];unsigned epoch;} DSResolution;
typedef struct {const DSResolution *owner;unsigned epoch,left,right;int status;DSMethodLease lease;} DSResolutionEntry;
typedef struct {DSResolutionEntry entries[2];unsigned next,hits,misses;} DSResolutionCache;
void ds_resolution_init(DSResolution *);
int ds_resolution_set(DSResolution *,unsigned,unsigned,DSCombine,DSLoaded *);
void ds_resolution_clear(DSResolution *);
int ds_resolution_uncached(const DSResolution *,unsigned,unsigned,int,int,int *);
void ds_resolution_cache_init(DSResolutionCache *);
void ds_resolution_cache_clear(DSResolutionCache *);
int ds_resolution_call(const DSResolution *,DSResolutionCache *,unsigned,unsigned,int,int,int *);
#endif
