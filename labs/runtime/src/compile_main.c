#include "cvm.h"
#include <stdio.h>
int main(int argc, char **argv) {
  char source[4097];
  uint8_t bytes[1100];
  size_t n, written;
  FILE *file;
  struct cvm_program program;
  enum cvm_status status;
  if (argc != 3) {
    fprintf(stderr, "usage: cvm-compile SOURCE OUTPUT.cvm\n");
    return 2;
  }
  file = fopen(argv[1], "rb");
  if (!file) {
    perror("source");
    return 1;
  }
  n = fread(source, 1, sizeof source, file);
  if (ferror(file)) {
    fclose(file);
    return 1;
  }
  fclose(file);
  if (n > 4096) {
    fprintf(stderr, "source capacity exceeded\n");
    return 1;
  }
  status = cvm_compile(source, n, &program);
  if (status == CVM_OK)
    status = cvm_save(&program, bytes, sizeof bytes, &written);
  if (status != CVM_OK) {
    fprintf(stderr, "%s\n", cvm_status_name(status));
    return 1;
  }
  file = fopen(argv[2], "wbx");
  if (!file) {
    perror("new output");
    return 1;
  }
  if (fwrite(bytes, 1, written, file) != written) {
    fclose(file);
    return 1;
  }
  if (fclose(file))
    return 1;
  printf("CVM1 bytes=%zu functions=%u\n", written, (unsigned)program.count);
  return 0;
}
