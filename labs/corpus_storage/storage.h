#ifndef CORPUS_STORAGE_H
#define CORPUS_STORAGE_H
#include <stddef.h>
#include <stdint.h>
enum ss_status { SS_OK,SS_MISSING,SS_FULL,SS_INVALID,SS_FAILED,SS_STALE,SS_EXHAUSTED };
struct ss_skip_node { uint32_t key; int32_t value; unsigned height,next[6],live; };
struct ss_skip { struct ss_skip_node nodes[32]; unsigned head[6],count; };
void ss_skip_init(struct ss_skip *);
unsigned ss_height(uint32_t,unsigned);
int ss_skip_put(struct ss_skip *,uint32_t,int32_t,uint32_t,unsigned);
int ss_skip_get(const struct ss_skip *,uint32_t,int32_t *,unsigned *);
int ss_skip_erase(struct ss_skip *,uint32_t);
int ss_skip_scan(const struct ss_skip *,uint32_t,uint32_t,uint32_t *,int32_t *,size_t,size_t *);
/* Deterministic noncryptographic LCG; next state is also the output. */
int ss_random(uint32_t *,uint32_t,unsigned,uint32_t *);
uint32_t ss_random_advance(uint32_t,uint32_t);

struct ss_page { uint32_t key[3]; int32_t value[3]; unsigned child[4],count,leaf,live,present[3]; };
struct ss_root { unsigned root,live; uint32_t generation; };
struct ss_tree { struct ss_page pages[256]; struct ss_root roots[8]; unsigned copies; };
struct ss_version { const struct ss_tree *owner; unsigned slot; uint32_t generation; };
void ss_tree_init(struct ss_tree *);
int ss_tree_snapshot(struct ss_tree *,struct ss_version *);
int ss_tree_release(struct ss_tree *,struct ss_version);
int ss_tree_get(const struct ss_tree *,const struct ss_version *,uint32_t,int32_t *);
int ss_tree_put(struct ss_tree *,uint32_t,int32_t,unsigned);
int ss_tree_erase(struct ss_tree *,uint32_t,unsigned);
int ss_tree_collect(struct ss_tree *,unsigned *);
int ss_tree_vacuum(struct ss_tree *,unsigned);

struct ss_record { uint32_t key,sequence; int32_t value; unsigned tombstone; };
struct ss_run { struct ss_record values[32]; unsigned count,live; };
struct ss_lsm { struct ss_record memory[4]; unsigned count; struct ss_run runs[3]; uint32_t sequence; unsigned written; };
void ss_lsm_init(struct ss_lsm *);
int ss_lsm_write(struct ss_lsm *,uint32_t,int32_t,unsigned);
int ss_lsm_get(const struct ss_lsm *,uint32_t,int32_t *,unsigned *);
int ss_lsm_flush(struct ss_lsm *,unsigned);
int ss_lsm_compact(struct ss_lsm *,unsigned,unsigned,unsigned);
#endif
