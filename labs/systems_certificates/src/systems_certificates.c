#include "systems_certificates.h"
enum sc_status sc_tree_check(const struct sc_tree *tree,unsigned *height){(void)tree;(void)height;return SC_UNIMPLEMENTED;}
enum sc_status sc_unique_insert(void *context,sc_contains_fn contains,sc_insert_fn insert,int32_t key){(void)context;(void)contains;(void)insert;(void)key;return SC_UNIMPLEMENTED;}
enum sc_status sc_matches(const uint8_t *text,size_t text_length,const uint8_t *pattern,size_t pattern_length,uint8_t *positions,size_t capacity,size_t *count){(void)text;(void)text_length;(void)pattern;(void)pattern_length;(void)positions;(void)capacity;(void)count;return SC_UNIMPLEMENTED;}
enum sc_status sc_clock(struct sc_machine *machine,const struct sc_instruction *program,size_t length,struct sc_control *control){(void)machine;(void)program;(void)length;(void)control;return SC_UNIMPLEMENTED;}
enum sc_status sc_run(struct sc_machine *machine,const struct sc_instruction *program,size_t length,unsigned budget,unsigned *executed){(void)machine;(void)program;(void)length;(void)budget;(void)executed;return SC_UNIMPLEMENTED;}
