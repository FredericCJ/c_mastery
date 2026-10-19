#include <stdio.h>
void su_note(const char *s) { puts(s); }
void su_values(const char *s,unsigned a,unsigned b,unsigned c,unsigned d) {
  printf("OBS %s %u %u %u %u\n",s,a,b,c,d);
}
int su_mapped(void);
int main(void) { return su_mapped(); }
