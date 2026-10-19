#include "verifier.h"
#ifdef VV_STANDALONE
int printf(const char *, ...);
int puts(const char *);
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
#else
#include <stdio.h>
#include <string.h>
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("VV_CHECK %u %s\n", (unsigned)__LINE__, #x);                      \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static VvWork work;
static CvmProgram program(const uint8_t *b, unsigned n, unsigned locals) {
  CvmProgram p = {0};
  p.count = 1;
  p.length = (uint16_t)n;
  p.functions[0] = (CvmFunction){0, (uint16_t)n, 0, (uint8_t)locals};
  memcpy(p.code, b, n);
  return p;
}
static int structural(void) {
  const uint8_t literal[] = {OP_INT, 42, 0, 0, 0, OP_RET};
  CvmProgram p = program(literal, 6, 0), out, before;
  VvReport r;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK && r.decoded == 2 &&
        r.discovered == 2 && r.processed == 2);
  uint8_t image[1100];
  size_t n;
  CHECK(cvm_encode(&p, image, sizeof image, &n) == CVM_OK);
  memset(&out, 91, sizeof out);
  memcpy(&before, &out, sizeof out);
  for (size_t i = 0; i < n; i++) {
    CHECK(vv_load(image, i, 0, 0, &work, &out, &r) == VV_LAYOUT);
    CHECK(!memcmp(&out, &before, sizeof out));
  }
  /* Unreachable malformed operands and targets still fail their own phase. */
  const uint8_t bad_width[] = {OP_NIL, OP_RET, OP_INT};
  p = program(bad_width, 3, 0);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_DECODE && r.pc == 2);
  const uint8_t dead_branch[] = {OP_NIL, OP_RET, OP_JMP, 3, 0};
  p = program(dead_branch, 5, 0);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_TARGET && r.pc == 2);
  p.code[3] = 0;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK && r.decoded == 3 &&
        r.discovered == 2);
  const uint8_t dead_split[] = {OP_NIL, OP_RET, VV_SPLIT, 0, 0, 3, 0};
  p = program(dead_split, 7, 0);
  CHECK(vv_verify(&p, 1, 0, &work, &r) == VV_TARGET && r.pc == 2);
  p.code[5] = 0;
  CHECK(vv_verify(&p, 1, 0, &work, &r) == VV_OK);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_DECODE); /* explicit dialect */
  const uint8_t wrong_local[] = {OP_LOAD, 1, OP_RET};
  p = program(wrong_local, 3, 1);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_DECODE);
  p.code[1] = 0;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK);
  const uint8_t cross[] = {OP_JMP, 3, 0, OP_NIL, OP_RET};
  p = program(cross, 5, 0);
  p.count = 2;
  p.functions[0].length = 3;
  p.functions[1] = (CvmFunction){3, 2, 0, 0};
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_TARGET);
  p.code[1] = 0;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK);
  const uint8_t call[] = {OP_CALL, 1, 1, OP_RET, OP_NIL, OP_RET};
  p = program(call, 6, 0);
  p.count = 2;
  p.functions[0].length = 4;
  p.functions[1] = (CvmFunction){4, 2, 0, 0};
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_DECODE);
  p.code[2] = 0;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK);
  const uint8_t backedge[] = {OP_NIL, OP_TRUE, OP_JFALSE, 8,      0, OP_JMP,
                              1,      0,       OP_NIL,    OP_JMP, 1, 0};
  p = program(backedge, 12, 0);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_HEIGHT && r.pc == 1);
  CHECK(cvm_encode(&p, image, sizeof image, &n) == CVM_OK);
  memcpy(&before, &out, sizeof out);
  CHECK(vv_load(image, n, 0, 0, &work, &out, &r) == VV_HEIGHT &&
        !memcmp(&before, &out, sizeof out));
  p.code[8] = OP_GC;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK && r.processed == r.discovered);
  const uint8_t fall[] = {OP_NIL};
  p = program(fall, 1, 0);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_TARGET);
  const uint8_t wrong_ret[] = {OP_NIL, OP_NIL, OP_RET};
  p = program(wrong_ret, 3, 0);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_HEIGHT);
  p = (CvmProgram){0};
  p.count = 1;
  p.length = CVM_CODE;
  p.functions[0].length = CVM_CODE;
  for (unsigned i = 0; i < CVM_CODE - 2; i++)
    p.code[i] = OP_GC;
  p.code[CVM_CODE - 2] = OP_NIL;
  p.code[CVM_CODE - 1] = OP_RET;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK && r.decoded == CVM_CODE &&
        r.processed == CVM_CODE && r.discovered == CVM_CODE &&
        r.max_queue == 1);
  CHECK(vv_verify(&p, 0, 1, &work, &r) == VV_OK && r.local_changes == 0 &&
        r.processed == CVM_CODE);
  p.functions[0].entry = 1;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_LAYOUT);
  CHECK(vv_verify(0, 0, 0, &work, &r) == VV_ARGUMENT);
  CHECK(vv_verify(&p, 2, 0, &work, &r) == VV_ARGUMENT);
  puts(
      "VV_STRUCTURE_PASS "
      "all_instruction_decode_dead_targets_nearest_valid_atomic_load_backedge");
  return 0;
}
static int initialized(void) {
  /* Short initialized path reaches LOAD before the long uninitialized path.
   * Its later intersection must propagate through an already processed node. */
  uint8_t b[32] = {OP_TRUE, OP_JFALSE, 14,       0, OP_INT, 42, 0,
                   0,       0,         OP_STORE, 0, OP_JMP, 29, 0};
  for (unsigned i = 14; i < 26; i++)
    b[i] = OP_GC;
  b[26] = OP_JMP;
  b[27] = 29;
  b[28] = 0;
  b[29] = OP_LOAD;
  b[30] = 0;
  b[31] = OP_RET;
  CvmProgram p = program(b, 32, 1);
  VvReport r;
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK);
  CHECK(vv_verify(&p, 0, 1, &work, &r) == VV_INITIALIZED && r.pc == 29 &&
        r.local_changes > 0);
  /* Replace the long arm's first three bytes with NIL/STORE0. */
  p.code[14] = OP_NIL;
  p.code[15] = OP_STORE;
  p.code[16] = 0;
  CHECK(vv_verify(&p, 0, 1, &work, &r) == VV_OK && work.initialized[29] == 1);
  /* Initialized sets do not prove value tags: NIL can be definitely assigned.
   */
  const uint8_t load[] = {OP_LOAD, 0, OP_RET};
  p = program(load, 3, 1);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK &&
        vv_verify(&p, 0, 1, &work, &r) == VV_INITIALIZED);
  /* A parameter is initialized, including for separately analyzed callees. */
  const uint8_t params[] = {OP_NIL, OP_CALL, 1, 1, OP_RET, OP_LOAD, 0, OP_RET};
  p = program(params, 8, 0);
  p.count = 2;
  p.functions[0].length = 5;
  p.functions[1] = (CvmFunction){5, 3, 1, 1};
  CHECK(vv_verify(&p, 0, 1, &work, &r) == VV_OK && work.initialized[5] == 1);
  puts("VV_INITIALIZED_PASS "
       "intersection_late_reprocessing_parameters_not_types");
  return 0;
}
static int lowering(void) {
  const uint8_t split[] = {OP_TRUE, VV_SPLIT, 6,  0, 12, 0, OP_INT, 42, 0, 0, 0,
                           OP_RET,  OP_INT,   17, 0, 0,  0, OP_RET};
  CvmProgram p = program(split, 18, 0), q, before;
  VvReport r;
  Cvm v;
  CvmValue result;
  for (unsigned truth = 0; truth < 2; truth++) {
    p.code[0] = (uint8_t)(truth ? OP_TRUE : OP_FALSE);
    CHECK(vv_verify(&p, 1, 0, &work, &r) == VV_OK);
    CHECK(cvm_verify(&p) == CVM_VERIFY);
    CHECK(vv_lower(&p, 1, &work, &q, &r) == VV_OK && q.length == 19 &&
          q.code[1] == OP_JFALSE && q.code[2] == 13 && q.code[4] == OP_JMP &&
          q.code[5] == 7);
    cvm_init(&v, 20, 0, 0);
    CHECK(cvm_run(&v, &q, &result) == CVM_OK && result.tag == CVM_INT &&
          result.integer == (truth ? 42 : 17));
  }
  /* Rebase later function descriptors and its internal branch as well. */
  const uint8_t multi[] = {OP_TRUE, VV_SPLIT, 6,      0,      6,     0, OP_CALL,
                           1,       0,        OP_RET, OP_JMP, 13,    0, OP_INT,
                           7,       0,        0,      0,      OP_RET};
  p = program(multi, 19, 0);
  p.count = 2;
  p.functions[0].length = 10;
  p.functions[1] = (CvmFunction){10, 9, 0, 0};
  CHECK(vv_lower(&p, 1, &work, &q, &r) == VV_OK && q.functions[1].entry == 11 &&
        q.code[12] == 14);
  cvm_init(&v, 20, 0, 0);
  CHECK(cvm_run(&v, &q, &result) == CVM_OK && result.integer == 7);
  /* Dead but structurally valid SPLIT expansion still consumes image space. */
  p = (CvmProgram){0};
  p.count = 1;
  p.length = 1022;
  p.functions[0].length = 1022;
  p.code[0] = OP_NIL;
  p.code[1] = OP_RET;
  for (unsigned pc = 2; pc < 1022; pc += 5) {
    p.code[pc] = VV_SPLIT;
  }
  CHECK(vv_verify(&p, 1, 0, &work, &r) == VV_OK);
  memset(&q, 91, sizeof q);
  memcpy(&before, &q, sizeof q);
  CHECK(vv_lower(&p, 1, &work, &q, &r) == VV_SPACE &&
        !memcmp(&q, &before, sizeof q));
  puts("VV_LOWER_PASS "
       "both_arms_actual_CVM_relocation_multifunction_failure_atomic_limit");
  return 0;
}
static int limits(void) {
  VvReport r;
  Cvm v;
  CvmValue out;
  CvmProgram p;
  const uint8_t uninit[] = {OP_LOAD, 0, OP_RET};
  p = program(uninit, 3, 1);
  CHECK(vv_verify(&p, 0, 0, &work, &r) == VV_OK && cvm_verify(&p) == CVM_OK);
  cvm_init(&v, 20, 0, 0);
  out = cvm_int(91);
  CHECK(cvm_run(&v, &p, &out) == CVM_TYPE && out.integer == 91);
  const uint8_t tag[] = {OP_TRUE, OP_FALSE, OP_ADD, OP_RET};
  p = program(tag, 4, 0);
  CHECK(vv_verify(&p, 0, 1, &work, &r) == VV_OK);
  cvm_init(&v, 20, 0, 0);
  CHECK(cvm_run(&v, &p, &out) == CVM_TYPE && out.integer == 91);
  const uint8_t loop[] = {OP_JMP, 0, 0};
  p = program(loop, 3, 0);
  CHECK(vv_verify(&p, 0, 1, &work, &r) == VV_OK);
  cvm_init(&v, 7, 0, 0);
  CHECK(cvm_run(&v, &p, &out) == CVM_FUEL && v.fuel == 0 && out.integer == 91);
  p = (CvmProgram){0};
  p.count = 2;
  unsigned at = 0;
  for (unsigned i = 0; i < 63; i++)
    p.code[at++] = OP_NIL;
  p.code[at++] = OP_CALL;
  p.code[at++] = 1;
  p.code[at++] = 0;
  for (unsigned i = 0; i < 63; i++)
    p.code[at++] = OP_POP;
  p.code[at++] = OP_RET;
  p.functions[0].length = (uint16_t)at;
  p.functions[1] = (CvmFunction){(uint16_t)at, 4, 0, 0};
  p.code[at++] = OP_NIL;
  p.code[at++] = OP_NIL;
  p.code[at++] = OP_POP;
  p.code[at++] = OP_RET;
  p.length = (uint16_t)at;
  CHECK(vv_verify(&p, 0, 1, &work, &r) == VV_OK && cvm_verify(&p) == CVM_OK);
  cvm_init(&v, 200, 0, 0);
  CHECK(cvm_run(&v, &p, &out) == CVM_STACK_LIMIT && out.integer == 91 &&
        v.sp == 64);
  puts("VV_LIMITS_PASS actual_TYPE_FUEL_STACK_LIMIT_no_result_publication");
  return 0;
}
int main(void) {
  CHECK(!structural());
  CHECK(!initialized());
  CHECK(!lowering());
  CHECK(!limits());
  puts("VV_PASS two_pass_lattice_lowering_actual_runtime");
  return 0;
}
