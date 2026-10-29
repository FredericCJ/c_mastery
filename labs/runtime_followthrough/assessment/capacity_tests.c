#include "cvm.h"
#include <stdio.h>
#define CHECK(x)                                                    \
    do                                                              \
    {                                                               \
        if (!(x))                                                   \
        {                                                           \
            printf("VP_CAP_CHECK %u %s\n", (unsigned)__LINE__, #x); \
            return 1;                                               \
        }                                                           \
    } while (0)

int main(void)
{
    struct cvm_cvm vm;
    struct cvm_value first, last, tmp;
    cvm_init(&vm, 0, 0, 0);
    vm.arena = 1;
    for (unsigned i = 0; i < CVM_HEAP; i++)
    {
        CHECK(cvm_pair(&vm, cvm_int((int32_t)i), cvm_int(0), &tmp) == CVM_OK);
        if (!i)
            first = tmp;
        last = tmp;
    }
    CHECK(cvm_pair(&vm, cvm_int(0), cvm_int(0), &tmp) == CVM_HEAP_LIMIT);
    vm.arena = 0;
    CHECK(cvm_root_push(&vm, first) == CVM_OK &&
          cvm_root_push(&vm, last) == CVM_OK);
    vm.gc_budget = CVM_HEAP + 5;
    CHECK(cvm_collect(&vm) == CVM_GC_BUDGET && vm.collections == 0 &&
          vm.gc_budget == CVM_HEAP - 1);
    unsigned used = 0;
    for (unsigned i = 0; i < CVM_HEAP; i++)
        used += vm.heap[i].used;
    CHECK(used == CVM_HEAP);
    vm.gc_budget = CVM_HEAP + 6;
    CHECK(cvm_collect(&vm) == CVM_OK && vm.gc_budget == 0 &&
          vm.collections == 1);
    used = 0;
    for (unsigned i = 0; i < CVM_HEAP; i++)
        used += vm.heap[i].used;
    CHECK(used == 2);
    CHECK(cvm_field(&vm, first, 0, &tmp) == CVM_OK && tmp.integer == 0);
    CHECK(cvm_field(&vm, last, 0, &tmp) == CVM_OK &&
          tmp.integer == (int32_t)CVM_HEAP - 1);
    printf("VP_CAP_PASS heap=%u root2_fields4_sweep=%u machine_bytes=%zu\n",
           CVM_HEAP, CVM_HEAP + 6, sizeof vm);
    return 0;
}
