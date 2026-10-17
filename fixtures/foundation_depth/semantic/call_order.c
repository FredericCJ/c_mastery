#include <stdio.h>
static int emit(int c) { putchar(c); return 1; }
int main(void) { return emit('A') + emit('B') == 2 ? 0 : 1; }
