#ifndef CMASTERY_CVM_H
#define CMASTERY_CVM_H
#include <stddef.h>
#include <stdint.h>
/* Original educational CVM1 contract; compiler is hosted, this core is
 * freestanding. */
#define CVM_CODE 1024u
#define CVM_FUNCTIONS 8u
#define CVM_STACK 64u
#define CVM_FRAMES 16u
#define CVM_LOCALS 8u
#define CVM_HEAP 16u
#define CVM_ROOTS 8u
typedef enum {
  CVM_OK,
  CVM_FORMAT,
  CVM_VERIFY,
  CVM_TYPE,
  CVM_RANGE,
  CVM_STACK_LIMIT,
  CVM_FRAME_LIMIT,
  CVM_FUEL,
  CVM_HEAP_LIMIT,
  CVM_GC_BUDGET,
  CVM_HOST_BUDGET,
  CVM_HOST_ERROR,
  CVM_STALE,
  CVM_SYNTAX,
  CVM_NOT_IMPLEMENTED
} CvmStatus;
typedef enum { CVM_NIL, CVM_INT, CVM_BOOL, CVM_REF } CvmTag;
typedef struct {
  CvmTag tag;
  int32_t integer;
  uint32_t generation;
} CvmValue;
typedef enum {
  OP_INT = 1,
  OP_TRUE,
  OP_FALSE,
  OP_NIL,
  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_EQ,
  OP_LT,
  OP_LOAD,
  OP_STORE,
  OP_POP,
  OP_JMP,
  OP_JFALSE,
  OP_CALL,
  OP_RET,
  OP_PAIR,
  OP_FIRST,
  OP_SECOND,
  OP_GC,
  OP_HOST
} CvmOpcode;
typedef struct {
  uint16_t entry, length;
  uint8_t arity, locals;
} CvmFunction;
typedef struct {
  uint8_t code[CVM_CODE];
  uint16_t length;
  uint8_t count;
  CvmFunction functions[CVM_FUNCTIONS];
} CvmProgram;
typedef struct {
  uint8_t function;
  uint16_t ip, base;
  CvmValue locals[CVM_LOCALS];
  uint8_t initialized[CVM_LOCALS];
} CvmFrame;
typedef struct {
  uint8_t used, marked;
  uint32_t generation;
  CvmValue left, right;
} CvmObject;
/* Callback must terminate with its own finite I/O/deadline policy. No VM
   reentry. Returning nonzero is failure. Only a CVM_INT crosses this interface.
 */
typedef int (*CvmHost)(void *, uint8_t, int32_t, int32_t *);
typedef struct {
  CvmValue stack[CVM_STACK], roots[CVM_ROOTS];
  CvmFrame frames[CVM_FRAMES];
  CvmObject heap[CVM_HEAP];
  uint16_t sp;
  uint8_t depth, root_count, arena;
  uint32_t fuel, gc_budget, host_budget, allocations, collections;
  CvmHost host;
  void *host_context;
} Cvm;
CvmValue cvm_int(int32_t n);
void cvm_init(Cvm *vm, uint32_t fuel, uint32_t gc_budget, uint32_t host_budget);
/* Structural container codec only: bounded, contiguous function regions and
 * failure-atomic output. Decoded instructions are NOT verified for execution.
 * Use load/save for verified images, and run verifies again before dispatch. */
CvmStatus cvm_decode(const uint8_t *bytes, size_t length, CvmProgram *out);
CvmStatus cvm_encode(const CvmProgram *p, uint8_t *out, size_t capacity,
                     size_t *written);
CvmStatus cvm_load(const uint8_t *bytes, size_t length, CvmProgram *out);
CvmStatus cvm_save(const CvmProgram *p, uint8_t *out, size_t capacity,
                   size_t *written);
CvmStatus cvm_verify(const CvmProgram *p);
CvmStatus cvm_run(Cvm *vm, const CvmProgram *p, CvmValue *result);
CvmStatus cvm_collect(Cvm *vm);
CvmStatus cvm_pair(Cvm *vm, CvmValue left, CvmValue right, CvmValue *out);
CvmStatus cvm_root_push(Cvm *vm, CvmValue value);
void cvm_root_pop(Cvm *vm);
CvmStatus cvm_field(Cvm *vm, CvmValue ref, int right, CvmValue *out);
/* Hosted compiler: source is an explicit byte span; output unchanged on
 * failure. */
CvmStatus cvm_compile(const char *source, size_t length, CvmProgram *out);
const char *cvm_status_name(CvmStatus status);
#endif
