#ifndef CMASTERY_OBJECT_RUNTIME_H
#define CMASTERY_OBJECT_RUNTIME_H
#include <stddef.h>
#include <stdint.h>
enum or_status { OR_OK=0, OR_INVALID=1, OR_FULL=2, OR_STATE=3, OR_DONE=4, OR_MISS=5, OR_UNIMPLEMENTED=9 };
uint64_t or_number_bits(uint64_t ieee_bits);
uint64_t or_integer(int32_t value);
enum or_status or_integer_get(uint64_t value, int32_t *out);
enum or_status or_pointer_tag(void *pointer, unsigned tag, uintptr_t *out);
enum or_status or_pointer_untag(uintptr_t word, unsigned tag, void **out);
struct or_intern { unsigned char bytes[8][15]; uint8_t length[8], used; };
enum or_status or_intern(struct or_intern *table,const unsigned char *bytes,size_t length,int fail,uint8_t *symbol);
struct or_shape { uint8_t count, parent, added, keys[4]; };
struct or_shapes { struct or_shape shape[16]; uint8_t used; };
struct or_object { uint8_t shape; int32_t fields[4]; };
struct or_cache { const struct or_shapes *owner; uint8_t shape[2], key[2], offset[2], count, mega; uint32_t hits, misses; };
void or_shapes_init(struct or_shapes *shapes);
enum or_status or_property_set(struct or_shapes *shapes,struct or_object *object,uint8_t key,int32_t value);
enum or_status or_property_get(const struct or_shapes *shapes,const struct or_object *object,uint8_t key,struct or_cache *cache,int32_t *out);
struct or_graph { uint16_t live, roots; uint8_t edge[16][2], color[16], active; };
enum or_status or_mark_begin(struct or_graph *graph);
enum or_status or_mark_step(struct or_graph *graph,unsigned budget,unsigned *scanned);
enum or_status or_graph_store(struct or_graph *graph,unsigned owner,unsigned field,unsigned target);
enum or_status or_graph_root(struct or_graph *graph,unsigned target,int present);
struct or_cell { int32_t value; uint8_t edge[2], age; };
struct or_nursery { struct or_cell young[2][8],old[8]; uint8_t active,used,old_used,roots[4]; };
enum or_status or_minor(struct or_nursery *heap,unsigned young_capacity,unsigned old_capacity,unsigned *copied);
struct or_cards { uint8_t edge[8][2], dirty; };
enum or_status or_card_store(struct or_cards *cards,unsigned object,unsigned field,unsigned target);
enum or_status or_card_scan(struct or_cards *cards,uint16_t *young_roots,unsigned *fields_scanned);
struct or_payload { int32_t value; uint8_t edge[2]; };
struct or_compact { struct or_payload cell[16]; uint16_t live; uint8_t handle[16]; unsigned borrows; };
enum or_status or_compact(struct or_compact *heap,uint8_t forwarding[16]);
struct or_weak { struct or_graph graph; uint8_t key[4],value[4],weak[4]; uint16_t finalizable,finalized,pending; };
enum or_status or_weak_collect(struct or_weak *heap,uint16_t *reclaimed);
enum or_status or_final_next(struct or_weak *heap,unsigned *identity);
struct or_counts { uint16_t live,refs[16]; uint8_t edge[16][2]; };
enum or_status or_trial_delete(struct or_counts *heap,uint16_t candidates,uint16_t *reclaimed);
struct or_slot { uint8_t tag, base, offset; uint32_t scalar; };
struct or_frame { struct or_slot slot[8]; uint8_t initialized, root_map, at_safepoint; };
enum or_status or_safepoint(struct or_frame *frame,const uint8_t forwarding[16],const uint8_t lengths[16]);
#endif
