#include "compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int vc_test(unsigned);
static VcWork work;
static Cvm vm;
int main(int argc, char **argv) {
  if (argc == 2 && !strcmp(argv[1], "--resources")) {
    printf("VC_RESOURCES work=%zu report=%zu program=%zu nodes=%u frames=%u\n",
           sizeof(VcWork), sizeof(VcReport), sizeof(CvmProgram), VC_NODES,
           VC_DEPTH);
    return 0;
  }
  if (argc == 2)
    return vc_test((unsigned)strtoul(argv[1], 0, 10));
  if (argc != 6 || (strcmp(argv[1], "--program") && strcmp(argv[1], "--legacy")))
    return 2;
  VcOptions o = {256, (unsigned)strtoul(argv[5], 0, 10)};
  VcReport r;
  CvmProgram p;
  CvmValue v = cvm_int(-777);
  int rc = !strcmp(argv[1], "--legacy")
               ? cvm_compile(argv[2], strlen(argv[2]), &p)
               : vc_compile(argv[2], strlen(argv[2]), &o, &work, &p, &r);
  printf("VC_COMPILE %d\n", rc);
  if (rc)
    return 0;
  printf("VC_CODE %u ", p.length);
  for (unsigned i = 0; i < p.length; i++)
    printf("%02x", p.code[i]);
  putchar('\n');
  for (unsigned i = 0; i < p.count; i++)
    printf("VC_FN %u %u %u %u\n", p.functions[i].entry, p.functions[i].length,
           p.functions[i].arity, p.functions[i].locals);
  cvm_init(&vm, (uint32_t)strtoul(argv[3], 0, 10),
           (uint32_t)strtoul(argv[4], 0, 10), 100);
  rc = cvm_run(&vm, &p, &v);
  printf("VC_RESULT %d %u %ld %u\n", rc, (unsigned)v.tag, (long)v.integer,
         vm.fuel);
  return 0;
}
