#ifndef CMASTERY_VM_LIMIT_FIXTURES_H
#define CMASTERY_VM_LIMIT_FIXTURES_H
#include "cvm.h"

/* Independent fixed bytecode fixture: INT7 INT8 PAIR STORE0 GC LOAD0 FIRST
 * HOST1 RET. Nine instructions, one pair, one initialized local at GC. */
static struct cvm_program vl_composed(void)
{
    struct cvm_program p = {{OP_INT,  7,        0,       0,     0,
                             OP_INT,  8,        0,       0,     0,
                             OP_PAIR, OP_STORE, 0,       OP_GC, OP_LOAD,
                             0,       OP_FIRST, OP_HOST, 1,     OP_RET},
                            20,
                            1,
                            {{0, 20, 0, 1}}};
    return p;
}
#endif
