#include "fileio.h"
void li_init(struct li_copy *c) {
  if (c)
    *c = (struct li_copy){0};
}
enum li_status li_pump(struct li_copy *c, LIRead read, void *rc, FWriter write,
                       void *wc, unsigned budget) {
  if (!c || !read || !write || c->offset > c->length || c->length > 2 ||
      c->written_total > c->read_total ||
      c->read_total - c->written_total != c->length - c->offset)
    return LI_ARGUMENT;
  if (c->terminal != LI_PENDING)
    return c->terminal;
  while (budget--) {
    if (c->offset < c->length) {
      size_t accepted = 0;
      FStatus s = f_write_all(write, wc, c->bytes + c->offset,
                              c->length - c->offset, 1, &accepted);
      if (accepted > SIZE_MAX - c->written_total)
        return c->terminal = LI_PROTOCOL;
#ifndef LI_MUTANT_REPLAY
      c->offset += accepted;
#endif
      c->written_total += accepted;
      if (s != F_OK && s != F_PENDING)
        return c->terminal = LI_WRITE_ERROR;
      if (c->offset < c->length)
        continue;
    } else {
      if (c->after_read == LI_EOF)
        return c->terminal = LI_DONE;
      if (c->after_read == LI_ERROR)
        return c->terminal = LI_READ_ERROR;
      struct li_read_result r = read(rc, c->bytes, sizeof c->bytes);
      if (r.count > 2 || r.status < LI_MORE || r.status > LI_WAIT ||
          (r.status == LI_WAIT && r.count) ||
          (r.status == LI_MORE && !r.count) ||
          r.count > SIZE_MAX - c->read_total)
        return c->terminal = LI_PROTOCOL;
      c->read_total += r.count;
      c->length = r.count;
      c->offset = 0;
      c->after_read = r.status;
#ifdef LI_MUTANT_ERROR
      if (r.status == LI_ERROR)
        c->after_read = LI_EOF;
#endif
      if (r.status == LI_WAIT)
        return LI_PENDING;
    }
  }
  return LI_PENDING;
}
enum li_status li_release(struct li_owner *o, int (*flush)(void *),
                          int (*close)(void *)) {
  if (!o)
    return LI_ARGUMENT;
  if (!o->handle)
    return LI_CLOSED;
  if ((o->owned && !close) || (!o->owned && o->output && !flush))
    return LI_ARGUMENT;
  void *handle = o->handle;
#ifndef LI_MUTANT_CLOSE
  o->handle = NULL;
#endif
  if (o->owned)
    return close(handle) ? LI_CLOSE_ERROR : LI_DONE;
  if (o->output)
    return flush(handle) ? LI_FLUSH_ERROR : LI_DONE;
  return LI_DONE;
}
