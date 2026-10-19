#ifndef BF_ARENA_H
#define BF_ARENA_H
#include "depth.h"
enum { BF_ARENA_BUSY=7, BF_ARENA_CLEANUP=8 };
typedef int (*BfClose)(void *);
typedef struct BfArena BfArena;
typedef struct {const BfArena *owner;unsigned id;} BfBorrow;
struct BfArena {
 B2Arena raw;
 _Alignas(8) unsigned char bytes[64];
 struct {unsigned id;size_t offset,length;} objects[16];
 struct {BfClose close;void *context;unsigned done;} cleanup[8];
 size_t object_saved[4],cleanup_saved[4],count,cleanup_count;
 unsigned next_object,busy,pending;
 B2Mark pending_mark;
};
/* Initialize fresh storage; never reset a live owner by reinitializing it. */
void bf_arena_init(BfArena *);
int bf_arena_reserve(BfArena *,size_t,size_t,BfBorrow *);
int bf_arena_get(const BfArena *,BfBorrow,size_t,unsigned char *);
int bf_arena_set(BfArena *,BfBorrow,size_t,unsigned char);
int bf_arena_mark(BfArena *,B2Mark *);
int bf_arena_register(BfArena *,BfClose,void *);
int bf_arena_rollback(BfArena *,B2Mark);
int bf_arena_commit(BfArena *,B2Mark);
#endif
