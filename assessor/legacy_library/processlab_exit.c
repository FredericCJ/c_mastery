#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int events;
static void a(void) {
  if (write(events, "A", 1) != 1)
    _Exit(3);
}
static void b(void) {
  if (write(events, "B", 1) != 1)
    _Exit(3);
}
static void x(void) {
  if (write(events, "X", 1) != 1)
    _Exit(3);
}
static void y(void) {
  if (write(events, "Y", 1) != 1)
    _Exit(3);
}
int main(int argc, char **argv) {
  static char buffer[128];
  if (argc != 3)
    return 2;
  events = open(argv[2], O_CREAT | O_EXCL | O_WRONLY, 0600);
  if (events < 0 || setvbuf(stdout, buffer, _IOFBF, sizeof buffer) ||
      atexit(a) || atexit(b) || at_quick_exit(x) || at_quick_exit(y))
    return 2;
  if (fputs("buffered:", stdout) < 0)
    return 2;
  if (!strcmp(argv[1], "normal"))
    exit(0);
  if (!strcmp(argv[1], "quick"))
    quick_exit(0);
  if (!strcmp(argv[1], "immediate"))
    _Exit(0);
  return 2;
}
