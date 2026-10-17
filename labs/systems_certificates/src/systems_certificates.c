#include "systems_certificates.h"
SCStatus sc_tree_check(const SCTree *tree,unsigned *height){(void)tree;(void)height;return SC_UNIMPLEMENTED;}
SCStatus sc_unique_insert(void *context,SCContains contains,SCInsert insert,int32_t key){(void)context;(void)contains;(void)insert;(void)key;return SC_UNIMPLEMENTED;}
SCStatus sc_matches(const uint8_t *text,size_t text_length,const uint8_t *pattern,size_t pattern_length,uint8_t *positions,size_t capacity,size_t *count){(void)text;(void)text_length;(void)pattern;(void)pattern_length;(void)positions;(void)capacity;(void)count;return SC_UNIMPLEMENTED;}
SCStatus sc_clock(SCMachine *machine,const SCInstruction *program,size_t length,SCControl *control){(void)machine;(void)program;(void)length;(void)control;return SC_UNIMPLEMENTED;}
SCStatus sc_run(SCMachine *machine,const SCInstruction *program,size_t length,unsigned budget,unsigned *executed){(void)machine;(void)program;(void)length;(void)budget;(void)executed;return SC_UNIMPLEMENTED;}
