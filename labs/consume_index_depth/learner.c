#include "consume_index.h"
unsigned ci_pointer(_Atomic(CINode *) *p) { (void)p; return 0; }
unsigned ci_value_index(_Atomic(CINode *) *p, const unsigned a[4]) {
    (void)p; (void)a; return 0;
}
unsigned ci_branch_index(_Atomic(CINode *) *p, const unsigned a[4]) {
    (void)p; (void)a; return 0;
}
unsigned ci_killed_index(_Atomic(CINode *) *p, const unsigned a[4]) {
    (void)p; (void)a; return 0;
}
unsigned ci_acquire_branch(_Atomic(CINode *) *p, const unsigned a[4]) {
    (void)p; (void)a; return 0;
}
unsigned ci_dependency_graph(unsigned code) { (void)code; return 0; }
