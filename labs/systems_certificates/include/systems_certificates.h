#ifndef CMASTERY_SYSTEMS_CERTIFICATES_H
#define CMASTERY_SYSTEMS_CERTIFICATES_H
#include <stddef.h>
#include <stdint.h>
enum sc_status {SC_OK=0,SC_INVALID=1,SC_FULL=2,SC_DUPLICATE=3,SC_DONE=4,SC_BUDGET=5,SC_UNIMPLEMENTED=9};
struct sc_node {int32_t key;int16_t left,right,parent;uint8_t height,live;};
struct sc_tree {struct sc_node nodes[64];int16_t root;uint8_t count,balanced;};
enum sc_status sc_tree_check(const struct sc_tree *tree,unsigned *height);
typedef int (*sc_contains_fn)(void *,int32_t);
typedef int (*sc_insert_fn)(void *,int32_t);
enum sc_status sc_unique_insert(void *context,sc_contains_fn contains,sc_insert_fn insert,int32_t key);
enum sc_status sc_matches(const uint8_t *text,size_t text_length,const uint8_t *pattern,size_t pattern_length,uint8_t *positions,size_t capacity,size_t *count);
struct sc_instruction {uint8_t operation,a,b,c;};
struct sc_machine {uint8_t reg[4],memory[16],pc,halted;uint32_t commits;};
struct sc_control {uint8_t pc_before,pc_after,reg_write,mem_write,branch_taken,alu;};
enum {SC_LI=0,SC_SUB=1,SC_BNZ=2,SC_ST=3,SC_LD=4,SC_HALT=5};
enum sc_status sc_clock(struct sc_machine *machine,const struct sc_instruction *program,size_t length,struct sc_control *control);
enum sc_status sc_run(struct sc_machine *machine,const struct sc_instruction *program,size_t length,unsigned budget,unsigned *executed);
#endif
