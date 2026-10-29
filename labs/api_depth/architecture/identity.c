#include "ar_build.h"
#include "arch.h"
#include <stdio.h>
int main(void) {
  printf("%s %s %s %d %zu\n", AR_SOURCE_ID, AR_TOOL_ID, AR_CONFIG_ID, AR_CAP,
         sizeof(struct ar_config));
  return 0;
}
