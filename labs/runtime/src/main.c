#include "cvm.h"
#include <stdio.h>
#include <string.h>
int main(int argc, char **argv) {
  Cvm vm;
  CvmProgram p;
  CvmValue result;
  CvmStatus s;
  if (argc != 2) {
    fprintf(stderr, "usage: cvm-run '(fn main () (+ 20 22))'\n");
    return 2;
  }
  s = cvm_compile(argv[1], strlen(argv[1]), &p);
  if (s == CVM_OK) {
    cvm_init(&vm, 10000, 10000, 0);
    s = cvm_run(&vm, &p, &result);
  }
  if (s != CVM_OK) {
    fprintf(stderr, "%s\n", cvm_status_name(s));
    return 1;
  }
  printf("tag=%d value=%ld\n", (int)result.tag, (long)result.integer);
  return 0;
}
