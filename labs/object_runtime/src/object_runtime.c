#include "object_runtime.h"
uint64_t or_number_bits(uint64_t ieee_bits) {(void)ieee_bits;return UINT64_C(0);}
uint64_t or_integer(int32_t value) {(void)value;return UINT64_C(0);}
enum or_status or_integer_get(uint64_t value, int32_t *out) {(void)value;(void)out;return OR_UNIMPLEMENTED;}
enum or_status or_pointer_tag(void *pointer, unsigned tag, uintptr_t *out) {(void)pointer;(void)tag;(void)out;return OR_UNIMPLEMENTED;}
enum or_status or_pointer_untag(uintptr_t word, unsigned tag, void **out) {(void)word;(void)tag;(void)out;return OR_UNIMPLEMENTED;}
enum or_status or_intern(struct or_intern *table,const unsigned char *bytes,size_t length,int fail,uint8_t *symbol) {(void)table;(void)bytes;(void)length;(void)fail;(void)symbol;return OR_UNIMPLEMENTED;}
void or_shapes_init(struct or_shapes *shapes) {(void)shapes;}
enum or_status or_property_set(struct or_shapes *shapes,struct or_object *object,uint8_t key,int32_t value) {(void)shapes;(void)object;(void)key;(void)value;return OR_UNIMPLEMENTED;}
enum or_status or_property_get(const struct or_shapes *shapes,const struct or_object *object,uint8_t key,struct or_cache *cache,int32_t *out) {(void)shapes;(void)object;(void)key;(void)cache;(void)out;return OR_UNIMPLEMENTED;}
enum or_status or_mark_begin(struct or_graph *graph) {(void)graph;return OR_UNIMPLEMENTED;}
enum or_status or_mark_step(struct or_graph *graph,unsigned budget,unsigned *scanned) {(void)graph;(void)budget;(void)scanned;return OR_UNIMPLEMENTED;}
enum or_status or_graph_store(struct or_graph *graph,unsigned owner,unsigned field,unsigned target) {(void)graph;(void)owner;(void)field;(void)target;return OR_UNIMPLEMENTED;}
enum or_status or_graph_root(struct or_graph *graph,unsigned target,int present) {(void)graph;(void)target;(void)present;return OR_UNIMPLEMENTED;}
enum or_status or_minor(struct or_nursery *heap,unsigned young_capacity,unsigned old_capacity,unsigned *copied) {(void)heap;(void)young_capacity;(void)old_capacity;(void)copied;return OR_UNIMPLEMENTED;}
enum or_status or_card_store(struct or_cards *cards,unsigned object,unsigned field,unsigned target) {(void)cards;(void)object;(void)field;(void)target;return OR_UNIMPLEMENTED;}
enum or_status or_card_scan(struct or_cards *cards,uint16_t *young_roots,unsigned *fields_scanned) {(void)cards;(void)young_roots;(void)fields_scanned;return OR_UNIMPLEMENTED;}
enum or_status or_compact(struct or_compact *heap,uint8_t forwarding[16]) {(void)heap;(void)forwarding;return OR_UNIMPLEMENTED;}
enum or_status or_weak_collect(struct or_weak *heap,uint16_t *reclaimed) {(void)heap;(void)reclaimed;return OR_UNIMPLEMENTED;}
enum or_status or_final_next(struct or_weak *heap,unsigned *identity) {(void)heap;(void)identity;return OR_UNIMPLEMENTED;}
enum or_status or_trial_delete(struct or_counts *heap,uint16_t candidates,uint16_t *reclaimed) {(void)heap;(void)candidates;(void)reclaimed;return OR_UNIMPLEMENTED;}
enum or_status or_safepoint(struct or_frame *frame,const uint8_t forwarding[16],const uint8_t lengths[16]) {(void)frame;(void)forwarding;(void)lengths;return OR_UNIMPLEMENTED;}
