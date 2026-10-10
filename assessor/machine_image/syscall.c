static long raw_write(long fd, const char *p, unsigned long n) {
  long r;
  __asm__ volatile("syscall"
                   : "=a"(r)
                   : "a"(1L), "D"(fd), "S"(p), "d"(n)
                   : "rcx", "r11", "memory");
  return r;
}
long mi_write_all(long fd, const char *p, unsigned long n) {
  unsigned long done = 0;
  while (done < n) {
    long r = raw_write(fd, p + done, n - done);
    if (r == -4)
      continue;
    if (r < 0)
      return r;
    if (r == 0)
      return -5;
    done += (unsigned long)r;
  }
  return (long)done;
}
