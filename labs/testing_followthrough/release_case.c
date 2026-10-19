#include <assert.h>
#include <stdio.h>
#ifndef RELEASE_REPAIRED
#define RELEASE_REPAIRED 0
#endif
static unsigned acquired;
static int acquire(void){acquired=1;return 1;}
int main(void){
#if RELEASE_REPAIRED
 int ok=acquire();if(!ok)return 2;
#else
 assert(acquire());
#endif
 if(!acquired){fputs("RELEASE_CHECK:assert-side-effect-removed\n",stderr);return 1;}
 puts("RELEASE_PASS:acquired");return 0;
}
