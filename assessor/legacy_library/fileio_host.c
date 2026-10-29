#define _GNU_SOURCE
#include "fileio.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      fprintf(stderr, "FILEIO-CHECK: %s line%d errno%d\n", #x, __LINE__,       \
              errno);                                                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
struct file_reader {
  FILE *f;
  unsigned calls;
};
static struct li_read_result file_read(void *v, unsigned char *p, size_t n) {
  struct file_reader *r = v;
  ++r->calls;
  size_t k = fread(p, 1, n, r->f);
  return (struct li_read_result){k, ferror(r->f) ? LI_ERROR
                                    : k          ? LI_MORE
                                    : feof(r->f) ? LI_EOF
                                                 : LI_ERROR};
}
static ptrdiff_t file_write(void *v, const uint8_t *p, size_t n) {
  FILE *f = v;
  if (ferror(f))
    return -1;
  size_t k = fwrite(p, 1, n, f);
  return k ? (ptrdiff_t)k : -1;
}
static int file_close(void *v) { return fclose(v); }
static int file_flush(void *v) { return fflush(v); }
static enum li_status copy(struct li_copy *c, LIRead r, void *rc, f_writer_fn w,
                           void *wc) {
  enum li_status s = LI_PENDING;
  for (unsigned i = 0; i < 64 && s == LI_PENDING; ++i)
    s = li_pump(c, r, rc, w, wc, 1);
  return s;
}
struct cookie {
  size_t position;
  unsigned closes;
  int close_error;
};
static ssize_t faulty_read(void *v, char *p, size_t n) {
  struct cookie *c = v;
  const char data[] = "ab";
  if (c->position < 2) {
    size_t k = 2 - c->position;
    if (k > n)
      k = n;
    memcpy(p, data + c->position, k);
    c->position += k;
    return (ssize_t)k;
  }
  errno = EIO;
  return -1;
}
static ssize_t faulty_write(void *v, const char *p, size_t n) {
  struct cookie *c = v;
  (void)p;
  if (c->position == 0 && n) {
    c->position = 1;
    return 1;
  }
  errno = EIO;
  return 0;
}
static int cookie_close(void *v) {
  struct cookie *c = v;
  ++c->closes;
  if (c->close_error) {
    errno = EIO;
    return -1;
  }
  return 0;
}
static ptrdiff_t pipe_write(void *v, const uint8_t *p, size_t n) {
  int fd = *(int *)v;
  ssize_t k = write(fd, p, n);
  if (k < 0 && (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR))
    return -2;
  return k > 0 ? (ptrdiff_t)k : -1;
}
int main(int argc, char **argv) {
  if (argc == 2 && !strcmp(argv[1], "borrow")) {
    struct li_owner out = {stdout, 0, 1};
    CHECK(fputs("borrowed", stdout) >= 0);
    CHECK(li_release(&out, file_flush, file_close) == LI_DONE && !out.handle);
    CHECK(fputs("-alive\n", stdout) >= 0);
    return fflush(stdout) != 0;
  }
  FILE *in = tmpfile(), *out = tmpfile();
  CHECK(in && out);
  char ib[128], ob[128];
  CHECK(setvbuf(in, ib, _IOFBF, sizeof ib) == 0 &&
        setvbuf(out, ob, _IOFBF, sizeof ob) == 0);
  CHECK(fwide(in, 0) == 0 && fwide(out, 0) == 0);
  CHECK(fwrite("abc", 1, 3, in) == 3 && fwide(in, 0) < 0);
  CHECK(fseek(in, 0, SEEK_SET) == 0 && !feof(in));
  struct file_reader reader = {in, 0};
  struct li_copy c;
  li_init(&c);
  CHECK(copy(&c, file_read, &reader, file_write, out) == LI_DONE &&
        c.written_total == 3 && reader.calls == 3 && feof(in) && !ferror(in));
  CHECK(fwide(out, 0) < 0);
  CHECK(fflush(out) == 0 && fseek(out, 0, SEEK_SET) == 0);
  char bytes[8] = {0};
  CHECK(fread(bytes, 1, 3, out) == 3 && !memcmp(bytes, "abc", 3));
  struct li_owner a = {in, 1, 0}, b = {out, 1, 1};
  CHECK(li_release(&a, file_flush, file_close) == LI_DONE &&
        li_release(&b, file_flush, file_close) == LI_DONE);
  CHECK(li_release(&a, file_flush, file_close) == LI_CLOSED);
  /* Actual GNU custom streams inject finite errors, never invalid FILE
   * accesses. */
  struct cookie read_cookie = {0};
  in = fopencookie(
      &read_cookie, "r",
      (cookie_io_functions_t){.read = faulty_read, .close = cookie_close});
  out = tmpfile();
  CHECK(in && out);
  reader = (struct file_reader){in, 0};
  li_init(&c);
  CHECK(copy(&c, file_read, &reader, file_write, out) == LI_READ_ERROR &&
        c.written_total == 2 && ferror(in));
  CHECK(fseek(out, 0, SEEK_SET) == 0 && fread(bytes, 1, 2, out) == 2 &&
        !memcmp(bytes, "ab", 2));
  CHECK(fclose(in) == 0 && fclose(out) == 0 && read_cookie.closes == 1);
  struct cookie write_cookie = {0};
  in = tmpfile();
  out = fopencookie(
      &write_cookie, "w",
      (cookie_io_functions_t){.write = faulty_write, .close = cookie_close});
  CHECK(in && out);
  CHECK(setvbuf(out, NULL, _IONBF, 0) == 0);
  CHECK(fwrite("abc", 1, 3, in) == 3 && fseek(in, 0, SEEK_SET) == 0);
  reader = (struct file_reader){in, 0};
  li_init(&c);
  CHECK(copy(&c, file_read, &reader, file_write, out) == LI_WRITE_ERROR &&
        c.written_total == 1 && ferror(out));
  CHECK(fclose(in) == 0);
  (void)fclose(out);
  CHECK(write_cookie.closes == 1);
  struct cookie close_cookie = {.close_error = 1};
  out = fopencookie(
      &close_cookie, "w",
      (cookie_io_functions_t){.write = faulty_write, .close = cookie_close});
  CHECK(out);
  b = (struct li_owner){out, 1, 1};
  CHECK(li_release(&b, file_flush, file_close) == LI_CLOSE_ERROR && !b.handle &&
        close_cookie.closes == 1);
  CHECK(li_release(&b, file_flush, file_close) == LI_CLOSED &&
        close_cookie.closes == 1);
  /* ISO update transitions: output->input flush/position; input->output
   * position. */
  in = tmpfile();
  CHECK(in);
  CHECK(fwrite("abc", 1, 3, in) == 3 && fflush(in) == 0 &&
        fseek(in, 0, SEEK_SET) == 0);
  CHECK(fgetc(in) == 'a');
  CHECK(fseek(in, 0, SEEK_CUR) == 0 && fputc('X', in) == 'X');
  CHECK(fflush(in) == 0 && fseek(in, 0, SEEK_SET) == 0 &&
        fread(bytes, 1, 3, in) == 3 && !memcmp(bytes, "aXc", 3));
  CHECK(fclose(in) == 0);
  /* POSIX seekable input fflush is an explicit extension to ISO C. */
  in = tmpfile();
  CHECK(in);
  CHECK(fwrite("abcdef", 1, 6, in) == 6 && fseek(in, 0, SEEK_SET) == 0);
  int fd = fileno(in);
  CHECK(fd >= 0 && fgetc(in) == 'a');
  CHECK(fflush(in) == 0);
  CHECK(read(fd, bytes, 1) == 1 && bytes[0] == 'b');
  CHECK(fgetc(in) == 'c');
  CHECK(fflush(in) == 0);
  int secondfd = dup(fd);
  CHECK(secondfd >= 0);
  FILE *second = fdopen(secondfd, "r");
  CHECK(second);
  CHECK(fseeko(second, 3, SEEK_SET) == 0 && fgetc(second) == 'd');
  off_t position = ftello(second);
  CHECK(position == 4 && fflush(second) == 0);
  CHECK(fseeko(in, position, SEEK_SET) == 0 && fgetc(in) == 'e');
  CHECK(fflush(in) == 0);
  CHECK(fclose(second) == 0 && fclose(in) == 0);
  /* Owned nonseekable source/sink; actual EAGAIN, no seeks or output replay. */
  int source_pipe[2], sink_pipe[2];
  CHECK(pipe(source_pipe) == 0 && pipe(sink_pipe) == 0);
  CHECK(write(source_pipe[1], "abc", 3) == 3 && close(source_pipe[1]) == 0);
  in = fdopen(source_pipe[0], "r");
  CHECK(in);
  int flags = fcntl(sink_pipe[1], F_GETFL);
  CHECK(flags >= 0 && fcntl(sink_pipe[1], F_SETFL, flags | O_NONBLOCK) == 0);
  unsigned char filler[1024];
  memset(filler, 'F', sizeof filler);
  size_t filled = 0;
  ssize_t n;
  while ((n = write(sink_pipe[1], filler, sizeof filler)) > 0) {
    filled += (size_t)n;
  }
  CHECK(n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK) && filled > 0);
  reader = (struct file_reader){in, 0};
  li_init(&c);
  CHECK(li_pump(&c, file_read, &reader, pipe_write, &sink_pipe[1], 4) ==
            LI_PENDING &&
        c.read_total == 2 && c.written_total == 0);
  for (size_t got = 0; got < filled;) {
    n = read(sink_pipe[0], filler, sizeof filler);
    CHECK(n > 0);
    got += (size_t)n;
  }
  CHECK(copy(&c, file_read, &reader, pipe_write, &sink_pipe[1]) == LI_DONE &&
        c.written_total == 3);
  CHECK(close(sink_pipe[1]) == 0 &&
        read(sink_pipe[0], bytes, sizeof bytes) == 3 &&
        !memcmp(bytes, "abc", 3));
  CHECK(close(sink_pipe[0]) == 0 && fclose(in) == 0);
  puts("FILEIO-PASS:actual FILE orientation/error/close,active handles,and "
       "nonseekable EAGAIN resume");
  return 0;
}
