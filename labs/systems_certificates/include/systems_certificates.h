#ifndef CMASTERY_SYSTEMS_CERTIFICATES_H
#define CMASTERY_SYSTEMS_CERTIFICATES_H
#include <stddef.h>
#include <stdint.h>
typedef enum {SC_OK=0,SC_INVALID=1,SC_FULL=2,SC_DUPLICATE=3,SC_DONE=4,SC_BUDGET=5,SC_UNIMPLEMENTED=9} SCStatus;
typedef struct {int32_t key;int16_t left,right,parent;uint8_t height,live;} SCNode;
typedef struct {SCNode nodes[64];int16_t root;uint8_t count,balanced;} SCTree;
SCStatus sc_tree_check(const SCTree *tree,unsigned *height);
typedef int (*SCContains)(void *,int32_t);
typedef int (*SCInsert)(void *,int32_t);
SCStatus sc_unique_insert(void *context,SCContains contains,SCInsert insert,int32_t key);
SCStatus sc_matches(const uint8_t *text,size_t text_length,const uint8_t *pattern,size_t pattern_length,uint8_t *positions,size_t capacity,size_t *count);
typedef struct {uint8_t operation,a,b,c;} SCInstruction;
typedef struct {uint8_t reg[4],memory[16],pc,halted;uint32_t commits;} SCMachine;
typedef struct {uint8_t pc_before,pc_after,reg_write,mem_write,branch_taken,alu;} SCControl;
enum {SC_LI=0,SC_SUB=1,SC_BNZ=2,SC_ST=3,SC_LD=4,SC_HALT=5};
SCStatus sc_clock(SCMachine *machine,const SCInstruction *program,size_t length,SCControl *control);
SCStatus sc_run(SCMachine *machine,const SCInstruction *program,size_t length,unsigned budget,unsigned *executed);
#endif
