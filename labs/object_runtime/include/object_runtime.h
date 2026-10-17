#ifndef CMASTERY_OBJECT_RUNTIME_H
#define CMASTERY_OBJECT_RUNTIME_H
#include <stddef.h>
#include <stdint.h>
typedef enum { OR_OK=0, OR_INVALID=1, OR_FULL=2, OR_STATE=3, OR_DONE=4, OR_MISS=5, OR_UNIMPLEMENTED=9 } ORStatus;
uint64_t or_number_bits(uint64_t ieee_bits);
uint64_t or_integer(int32_t value);
ORStatus or_integer_get(uint64_t value, int32_t *out);
ORStatus or_pointer_tag(void *pointer, unsigned tag, uintptr_t *out);
ORStatus or_pointer_untag(uintptr_t word, unsigned tag, void **out);
typedef struct { unsigned char bytes[8][15]; uint8_t length[8], used; } ORIntern;
ORStatus or_intern(ORIntern *table,const unsigned char *bytes,size_t length,int fail,uint8_t *symbol);
typedef struct { uint8_t count, parent, added, keys[4]; } ORShape;
typedef struct { ORShape shape[16]; uint8_t used; } ORShapes;
typedef struct { uint8_t shape; int32_t fields[4]; } ORObject;
typedef struct { const ORShapes *owner; uint8_t shape[2], key[2], offset[2], count, mega; uint32_t hits, misses; } ORCache;
void or_shapes_init(ORShapes *shapes);
ORStatus or_property_set(ORShapes *shapes,ORObject *object,uint8_t key,int32_t value);
ORStatus or_property_get(const ORShapes *shapes,const ORObject *object,uint8_t key,ORCache *cache,int32_t *out);
typedef struct { uint16_t live, roots; uint8_t edge[16][2], color[16], active; } ORGraph;
ORStatus or_mark_begin(ORGraph *graph);
ORStatus or_mark_step(ORGraph *graph,unsigned budget,unsigned *scanned);
ORStatus or_graph_store(ORGraph *graph,unsigned owner,unsigned field,unsigned target);
ORStatus or_graph_root(ORGraph *graph,unsigned target,int present);
typedef struct { int32_t value; uint8_t edge[2], age; } ORCell;
typedef struct { ORCell young[2][8],old[8]; uint8_t active,used,old_used,roots[4]; } ORNursery;
ORStatus or_minor(ORNursery *heap,unsigned young_capacity,unsigned old_capacity,unsigned *copied);
typedef struct { uint8_t edge[8][2], dirty; } ORCards;
ORStatus or_card_store(ORCards *cards,unsigned object,unsigned field,unsigned target);
ORStatus or_card_scan(ORCards *cards,uint16_t *young_roots,unsigned *fields_scanned);
typedef struct { int32_t value; uint8_t edge[2]; } ORPayload;
typedef struct { ORPayload cell[16]; uint16_t live; uint8_t handle[16]; unsigned borrows; } ORCompact;
ORStatus or_compact(ORCompact *heap,uint8_t forwarding[16]);
typedef struct { ORGraph graph; uint8_t key[4],value[4],weak[4]; uint16_t finalizable,finalized,pending; } ORWeak;
ORStatus or_weak_collect(ORWeak *heap,uint16_t *reclaimed);
ORStatus or_final_next(ORWeak *heap,unsigned *identity);
typedef struct { uint16_t live,refs[16]; uint8_t edge[16][2]; } ORCounts;
ORStatus or_trial_delete(ORCounts *heap,uint16_t candidates,uint16_t *reclaimed);
typedef struct { uint8_t tag, base, offset; uint32_t scalar; } ORSlot;
typedef struct { ORSlot slot[8]; uint8_t initialized, root_map, at_safepoint; } ORFrame;
ORStatus or_safepoint(ORFrame *frame,const uint8_t forwarding[16],const uint8_t lengths[16]);
#endif
