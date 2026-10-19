#ifndef CORPUS_STORAGE_H
#define CORPUS_STORAGE_H
#include <stddef.h>
#include <stdint.h>
enum SsStatus { SS_OK,SS_MISSING,SS_FULL,SS_INVALID,SS_FAILED,SS_STALE,SS_EXHAUSTED };
typedef struct { uint32_t key; int32_t value; unsigned height,next[6],live; } SsSkipNode;
typedef struct { SsSkipNode nodes[32]; unsigned head[6],count; } SsSkip;
void ss_skip_init(SsSkip *);
unsigned ss_height(uint32_t,unsigned);
int ss_skip_put(SsSkip *,uint32_t,int32_t,uint32_t,unsigned);
int ss_skip_get(const SsSkip *,uint32_t,int32_t *,unsigned *);
int ss_skip_erase(SsSkip *,uint32_t);
int ss_skip_scan(const SsSkip *,uint32_t,uint32_t,uint32_t *,int32_t *,size_t,size_t *);
/* Deterministic noncryptographic LCG; next state is also the output. */
int ss_random(uint32_t *,uint32_t,unsigned,uint32_t *);
uint32_t ss_random_advance(uint32_t,uint32_t);

typedef struct { uint32_t key[3]; int32_t value[3]; unsigned child[4],count,leaf,live,present[3]; } SsPage;
typedef struct { unsigned root,live; uint32_t generation; } SsRoot;
typedef struct { SsPage pages[256]; SsRoot roots[8]; unsigned copies; } SsTree;
typedef struct { const SsTree *owner; unsigned slot; uint32_t generation; } SsVersion;
void ss_tree_init(SsTree *);
int ss_tree_snapshot(SsTree *,SsVersion *);
int ss_tree_release(SsTree *,SsVersion);
int ss_tree_get(const SsTree *,const SsVersion *,uint32_t,int32_t *);
int ss_tree_put(SsTree *,uint32_t,int32_t,unsigned);
int ss_tree_erase(SsTree *,uint32_t,unsigned);
int ss_tree_collect(SsTree *,unsigned *);
int ss_tree_vacuum(SsTree *,unsigned);

typedef struct { uint32_t key,sequence; int32_t value; unsigned tombstone; } SsRecord;
typedef struct { SsRecord values[32]; unsigned count,live; } SsRun;
typedef struct { SsRecord memory[4]; unsigned count; SsRun runs[3]; uint32_t sequence; unsigned written; } SsLsm;
void ss_lsm_init(SsLsm *);
int ss_lsm_write(SsLsm *,uint32_t,int32_t,unsigned);
int ss_lsm_get(const SsLsm *,uint32_t,int32_t *,unsigned *);
int ss_lsm_flush(SsLsm *,unsigned);
int ss_lsm_compact(SsLsm *,unsigned,unsigned,unsigned);
#endif
