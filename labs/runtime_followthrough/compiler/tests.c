#include "compiler.h"
#include <stdio.h>
#include <string.h>
static struct vc_work work;
static struct cvm_cvm vm;
static struct cvm_program program;
static struct vc_report report;
static unsigned effects;
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("VC_ASSERT:%s\n", #x);                                            \
      return 41;                                                               \
    }                                                                          \
  } while (0)
static int host(void *p, uint8_t id, int32_t value, int32_t *out) {
  (void)p;
  effects++;
  *out = value + (int32_t)id;
  return 0;
}
static int compile(const char *s, unsigned depth, unsigned fold) {
  struct vc_options o = {depth, fold};
  return vc_compile(s, strlen(s), &o, &work, &program, &report);
}
static int value(const char *s, unsigned fold, enum cvm_tag tag, int32_t expected) {
  CHECK(compile(s, 256, fold) == CVM_OK);
  struct cvm_value result = cvm_int(-777);
  cvm_init(&vm, 10000, 10000, 100);
  vm.host = host;
  vm.host_context = 0;
  CHECK(cvm_run(&vm, &program, &result) == CVM_OK && result.tag == tag &&
        result.integer == expected);
  return 0;
}
static int refusal(const char *s, size_t n, unsigned depth) {
  struct cvm_program prior;
  memset(&program, 0xa5, sizeof program);
  memcpy(&prior, &program, sizeof program);
  struct vc_options o = {depth, 0};
  CHECK(vc_compile(s, n, &o, &work, &program, &report) == CVM_SYNTAX);
  CHECK(!memcmp(&prior, &program, sizeof program));
  return 0;
}
static int names(void) {
  CHECK(!value("(fn main () (call f 2)) (fn f (x) (+ x 3))", 0, CVM_INT, 5));
  CHECK(!value("(fn main () (let x 2 (let x (+ x 1) x)))", 0, CVM_INT, 3));
  CHECK(!value(
      "(fn main () (let p (let a 11 (pair a a)) (do (let a 99 a) (first p))))",
      0, CVM_INT, 11));
  CHECK(report.local_peak == 2);
  const char *bad[] = {
      "(fn main () 1) (fn main () 2)",
      "(fn main () 1) (fn f (x x) x)",
      "(fn main () (call f 1 2)) (fn f (x) x)",
      "(fn main () (block (let x 2 x) x))",
      "(fn main () (let x x x))",
      "(fn main () (let abcdefghijklmnopqrstuvwxyzabcdef 1 1))"};
  for (unsigned i = 0; i < sizeof bad / sizeof bad[0]; i++)
    CHECK(!refusal(bad[i], strlen(bad[i]), 64));
  CHECK(!value("(fn main () (let abcdefghijklmnopqrstuvwxyzabcde 4 "
               "abcdefghijklmnopqrstuvwxyzabcde))",
               0, CVM_INT, 4));
  char embedded[] = "(fn main () 1)";
  embedded[11] = 0;
  CHECK(!refusal(embedded, sizeof embedded - 1, 64));
  puts("VC_VALUES names 5 3 11 2\nVC_NAMES_PASS");
  return 0;
}
static int scopes(void) {
  CHECK(!value("(fn main () (let x 2 (+ (let x 40 x) x)))", 0, CVM_INT, 42));
  CHECK(!value("(fn main () (let x 2 (+ (let x 40 (set x 41)) x)))", 0, CVM_INT,
               43));
  char s[4097];
  strcpy(s, "(fn main () (block");
  for (unsigned i = 0; i < 20; i++)
    strcat(s, " (let a 3 a)");
  strcat(s, "))");
  CHECK(!value(s, 0, CVM_INT, 3) && report.local_peak == 1);
  CHECK(!value(
      "(fn main () (let x 1 (do (block (let x 4 (set x 8)) (set x 9)) x)))", 0,
      CVM_INT, 9));
  puts("VC_VALUES scopes 42 43 1 9\nVC_SCOPES_PASS");
  return 0;
}
static void nesting(char *s, unsigned n) {
  strcpy(s, "(fn main () ");
  for (unsigned i = 0; i < n; i++)
    strcat(s, "(first ");
  strcat(s, "nil");
  for (unsigned i = 0; i < n; i++)
    strcat(s, ")");
  strcat(s, ")");
}
static int bounds(void) {
  char s[4098];
  nesting(s, 63);
  CHECK(compile(s, 64, 0) == CVM_OK && report.emitter_peak == 64);
  nesting(s, 64);
  CHECK(!refusal(s, strlen(s), 64));
  nesting(s, 255);
  CHECK(compile(s, 256, 0) == CVM_OK && report.emitter_peak == 256);
  nesting(s, 256);
  CHECK(!refusal(s, strlen(s), 256));
  strcpy(s, "(fn main () (block");
  for (unsigned i = 0; i < 512; i++)
    strcat(s, " true");
  strcat(s, "))");
  CHECK(compile(s, 64, 0) == CVM_OK && program.length == 1024);
  strcpy(s, "(fn main () (block");
  for (unsigned i = 0; i < 513; i++)
    strcat(s, " true");
  strcat(s, "))");
  CHECK(!refusal(s, strlen(s), 64));
  CHECK(
      !value("(fn main () (call f 1 2 3 4 5 6 7 8)) (fn f (a b c d e f g h) h)",
             0, CVM_INT, 8));
  const char *nine = "(fn main () 0) (fn f (a b c d e f g h i) a)";
  CHECK(!refusal(nine, strlen(nine), 64));
  memset(s, ' ', 4097);
  memcpy(s, "(fn main () 1)", 14);
  s[4097] = 0;
  CHECK(compile(s, 64, 0) == CVM_SYNTAX);
  s[4096] = 0;
  CHECK(compile(s, 64, 0) == CVM_OK);
  puts("VC_VALUES bounds 64 256 1024 8 4096\nVC_BOUNDS_PASS");
  return 0;
}
static int branches(void) {
  CHECK(!value("(fn main () (if false (if true 1 2) (if true 3 4)))", 0,
               CVM_INT, 3));
  CHECK(!value("(fn main () (and false (/ 1 0)))", 0, CVM_BOOL, 0));
  effects = 0;
  CHECK(!value("(fn main () (or true (host 0 1)))", 0, CVM_BOOL, 1) &&
        effects == 0);
  CHECK(!value("(fn main () (and true (or false true)))", 0, CVM_BOOL, 1));
  CHECK(compile("(fn main () (and true 3))", 64, 0) == CVM_OK);
  struct cvm_value out = cvm_int(99);
  cvm_init(&vm, 100, 100, 10);
  CHECK(cvm_run(&vm, &program, &out) == CVM_TYPE && out.integer == 99);
  /* Independent decoder establishes boundaries before a real invalid target. */
  CHECK(compile("(fn main () (if true 1 2))", 64, 0) == CVM_OK);
  unsigned pc = 0, jump = 0;
  unsigned char boundary[CVM_CODE] = {0};
  while (pc < program.length) {
    boundary[pc] = 1;
    unsigned op = program.code[pc],
             width = op == OP_INT                                         ? 5
                     : (op == OP_JMP || op == OP_JFALSE || op == OP_CALL) ? 3
                     : (op == OP_LOAD || op == OP_STORE || op == OP_HOST) ? 2
                                                                          : 1;
    if (op == OP_JFALSE)
      jump = pc;
    pc += width;
  }
  CHECK(pc == program.length && jump &&
        boundary[program.code[jump + 1] + 256u * program.code[jump + 2]]);
  program.code[jump + 1] = (uint8_t)(jump + 1);
  program.code[jump + 2] = 0;
  struct vv_report v;
  CHECK(vv_verify(&program, 0, 1, &work.verification, &v) == VV_TARGET);
  puts("VC_VALUES branches 3 0 1 0\nVC_BRANCHES_PASS");
  return 0;
}
static int controls(void) {
  effects = 0;
  CHECK(!value("(fn main () (if true (host 1 7) (host 2 8)))", 0, CVM_INT, 8) &&
        effects == 1);
  CHECK(!value("(fn main () (let x 1 (do (set x 7) x)))", 0, CVM_INT, 7));
  CHECK(!value("(fn main () (while false nil))", 0, CVM_NIL, 0));
  CHECK(!value("(fn main () (+ 100 (while true (+ 10 (let x 4 (break x))))))",
               0, CVM_INT, 104));
  CHECK(
      !value("(fn main () (while true (while (break 9) nil)))", 0, CVM_INT, 9));
  CHECK(!value("(fn main () (while true (break (pair 7 8))))", 0, CVM_REF, 0));
  CHECK(!value("(fn main () (first (while true (let x 7 (break (pair x 8))))))",
               0, CVM_INT, 7));
  const char *bad = "(fn main () (break 1))";
  CHECK(!refusal(bad, strlen(bad), 64));
  CHECK(compile("(fn main () (if 1 2 3))", 64, 0) == CVM_OK);
  struct cvm_value out = cvm_int(99);
  cvm_init(&vm, 100, 100, 10);
  CHECK(cvm_run(&vm, &program, &out) == CVM_TYPE && out.integer == 99);
  puts("VC_VALUES controls 8 1 7 104 9\nVC_CONTROLS_PASS");
  return 0;
}
static int fuel(void) {
  const char *counter =
      "(fn main () (let n 0 (do (while (< n 4) (set n (+ n 1))) n)))";
  CHECK(compile(counter, 64, 0) == CVM_OK);
  for (unsigned budget = 55; budget <= 57; budget++) {
    struct cvm_value out = cvm_int(99);
    cvm_init(&vm, budget, 100, 10);
    int rc = cvm_run(&vm, &program, &out);
    CHECK(budget == 55
              ? (rc == CVM_FUEL && out.integer == 99)
              : (rc == CVM_OK && out.integer == 4 && vm.fuel == budget - 56));
  }
  CHECK(compile("(fn main () (while true nil))", 64, 0) == CVM_OK);
  for (unsigned b = 0; b < 100; b++) {
    struct cvm_value out = cvm_int(99);
    cvm_init(&vm, b, 100, 10);
    CHECK(cvm_run(&vm, &program, &out) == CVM_FUEL && out.integer == 99 &&
          vm.fuel == 0);
  }
  CHECK(compile("(fn main () (+ 2 3))", 64, 0) == CVM_OK &&
        report.folded == 0 && program.length == 12);
  struct cvm_value out = cvm_int(99);
  cvm_init(&vm, 2, 100, 10);
  CHECK(cvm_run(&vm, &program, &out) == CVM_FUEL && out.integer == 99);
  CHECK(compile("(fn main () (+ 2 3))", 64, 1) == CVM_OK &&
        report.folded == 1 && program.length == 6);
  cvm_init(&vm, 2, 100, 10);
  CHECK(cvm_run(&vm, &program, &out) == CVM_OK && out.integer == 5 &&
        vm.fuel == 0);
  for (unsigned fold = 0; fold < 2; fold++) {
    CHECK(compile("(fn main () (+ 2147483647 1))", 64, fold) == CVM_OK &&
          report.folded == 0);
    cvm_init(&vm, 100, 100, 10);
    CHECK(cvm_run(&vm, &program, &out) == CVM_RANGE);
  }
  puts("VC_VALUES fuel 56 4 2 5\nVC_FUEL_PASS");
  return 0;
}
int vc_test(unsigned n) {
  switch (n) {
  case 0:
    return names();
  case 1:
    return scopes();
  case 2:
    return bounds();
  case 3:
    return branches();
  case 4:
    return controls();
  case 5:
    return fuel();
  default:
    return 41;
  }
}
