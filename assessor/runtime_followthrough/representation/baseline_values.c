#include "cvm.h"
#include <stdio.h>
static void integer(struct cvm_program *p) {
  p->code[p->length++] = OP_INT;
  for (unsigned i = 0; i < 4; i++)
    p->code[p->length++] = 0;
}
int main(void) {
  for (unsigned a = 0; a < 3; a++)
    for (unsigned b = 0; b < 3; b++) {
      struct cvm_program p = {0};
      struct cvm_cvm v;
      struct cvm_value out = cvm_int(91);
      p.count = 1;
      if (!a)
        integer(&p);
      else
        p.code[p.length++] = (uint8_t)(a == 1 ? OP_FALSE : OP_NIL);
      if (!b)
        integer(&p);
      else
        p.code[p.length++] = (uint8_t)(b == 1 ? OP_FALSE : OP_NIL);
      p.code[p.length++] = OP_EQ;
      p.code[p.length++] = OP_RET;
      p.functions[0].length = p.length;
      cvm_init(&v, 100, 0, 0);
      if (cvm_run(&v, &p, &out) != CVM_OK || out.tag != CVM_BOOL ||
          out.integer != (a == b))
        return 1;
    }
  struct cvm_program p = {0};
  struct cvm_cvm v;
  struct cvm_value out = cvm_int(91);
  p.count = 1;
  p.code[p.length++] = OP_TRUE;
  integer(&p);
  p.code[p.length++] = OP_ADD;
  p.code[p.length++] = OP_RET;
  p.functions[0].length = p.length;
  cvm_init(&v, 100, 0, 0);
  if (cvm_run(&v, &p, &out) != CVM_TYPE || out.tag != CVM_INT ||
      out.integer != 91)
    return 1;
  puts("RV_BASELINE_VALUES_PASS nine_tag_pairs Boolean_ADD_TYPE "
       "unchanged_result");
  return 0;
}
