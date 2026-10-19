#ifndef CMASTERY_BUILD_CACHE_H
#define CMASTERY_BUILD_CACHE_H
#include "metaprog.h"
enum { MPB_OK,MPB_INVALID,MPB_CYCLE,MPB_FULL,MPB_BUSY,MPB_DONE,MPB_FAILED,MPB_STALE,MPB_EXHAUSTED,MPB_IO,MPB_UNCERTAIN,MPB_MISS };
enum { MPB_SOURCE,MPB_TOOL,MPB_CONFIG,MPB_ENVIRONMENT,MPB_INCLUDE,MPB_SCHEMA };
typedef struct {unsigned char bytes[8];size_t length;} MpbBytes;
typedef struct {unsigned id,dependencies;int32_t revision;MpbBytes inputs[6];} MpbNode;
typedef struct {uint64_t key;unsigned char input[512];size_t input_length;char artifact[256];size_t artifact_length;} MpbEntry;
typedef struct {MpbEntry entries[8];unsigned order[8],dependencies[8];size_t count;} MpbPlan;
int mpb_plan(const MpbNode *nodes,size_t count,unsigned force_collision,MpbPlan *output);
int mpb_match(const MpbEntry *expected,const MpbEntry *cached);
int mpb_encode(const MpbEntry *entry,unsigned id,unsigned char *bytes,size_t capacity,size_t *used);
typedef struct MpbSchedule MpbSchedule;
typedef struct {const MpbSchedule *owner;uint64_t epoch;unsigned id;} MpbTicket;
struct MpbSchedule {const MpbPlan *plan;uint64_t epoch;unsigned pending,running,done,failed;};
int mpb_begin(MpbSchedule *schedule,const MpbPlan *plan);
int mpb_claim(MpbSchedule *schedule,MpbTicket *ticket);
int mpb_finish(MpbSchedule *schedule,MpbTicket ticket,const MpbEntry *produced);
int mpb_publish(const MpbSchedule *schedule,MpbPlan *output);
#endif
