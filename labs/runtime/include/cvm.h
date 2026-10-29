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

enum cvm_status
{
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
};

enum cvm_tag
{
    CVM_NIL,
    CVM_INT,
    CVM_BOOL,
    CVM_REF
};

struct cvm_value
{
    enum cvm_tag tag;
    int32_t integer;
    uint32_t generation;
};

enum cvm_opcode
{
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
};

struct cvm_function
{
    uint16_t entry, length;
    uint8_t arity, locals;
};

struct cvm_program
{
    uint8_t code[CVM_CODE];
    uint16_t length;
    uint8_t count;
    struct cvm_function functions[CVM_FUNCTIONS];
};

struct cvm_frame
{
    uint8_t function;
    uint16_t ip, base;
    struct cvm_value locals[CVM_LOCALS];
    uint8_t initialized[CVM_LOCALS];
};

struct cvm_object
{
    uint8_t used, marked;
    uint32_t generation;
    struct cvm_value left, right;
};

/* Callback must terminate with its own finite I/O/deadline policy. No VM
   reentry. Returning nonzero is failure. Only a CVM_INT crosses this interface.
 */
typedef int (*cvm_host_fn)(void *, uint8_t, int32_t, int32_t *);

struct cvm_cvm
{
    struct cvm_value stack[CVM_STACK], roots[CVM_ROOTS];
    struct cvm_frame frames[CVM_FRAMES];
    struct cvm_object heap[CVM_HEAP];
    uint16_t sp;
    uint8_t depth, root_count, arena;
    uint32_t fuel, gc_budget, host_budget, allocations, collections;
    cvm_host_fn host;
    void *host_context;
};
struct cvm_value cvm_int(int32_t n);
void cvm_init(struct cvm_cvm *vm, uint32_t fuel, uint32_t gc_budget,
              uint32_t host_budget);
/* Structural container codec only: bounded, contiguous function regions and
 * failure-atomic output. Decoded instructions are NOT verified for execution.
 * Use load/save for verified images, and run verifies again before dispatch. */
enum cvm_status cvm_decode(const uint8_t *bytes, size_t length,
                           struct cvm_program *out);
enum cvm_status cvm_encode(const struct cvm_program *p, uint8_t *out,
                           size_t capacity, size_t *written);
enum cvm_status cvm_load(const uint8_t *bytes, size_t length,
                         struct cvm_program *out);
enum cvm_status cvm_save(const struct cvm_program *p, uint8_t *out,
                         size_t capacity, size_t *written);
enum cvm_status cvm_verify(const struct cvm_program *p);
enum cvm_status cvm_run(struct cvm_cvm *vm, const struct cvm_program *p,
                        struct cvm_value *result);
enum cvm_status cvm_collect(struct cvm_cvm *vm);
enum cvm_status cvm_pair(struct cvm_cvm *vm, struct cvm_value left,
                         struct cvm_value right, struct cvm_value *out);
enum cvm_status cvm_root_push(struct cvm_cvm *vm, struct cvm_value value);
void cvm_root_pop(struct cvm_cvm *vm);
enum cvm_status cvm_field(struct cvm_cvm *vm, struct cvm_value ref, int right,
                          struct cvm_value *out);
/* Hosted compiler: source is an explicit byte span; output unchanged on
 * failure. */
enum cvm_status cvm_compile(const char *source, size_t length,
                            struct cvm_program *out);
const char *cvm_status_name(enum cvm_status status);
#endif
