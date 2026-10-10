#include "v4_foundation.h"
int v4_admit(V4Admission *s, uint32_t n) {
  if (!s || !n || n > s->available || n > UINT32_MAX - s->active)
    return -1;
  s->available -= n;
  s->active += n;
  return 0;
}
int v4_release(V4Admission *s, uint32_t n) {
  if (!s || !n || n > s->active || n > UINT32_MAX - s->available)
    return -1;
  s->active -= n;
  s->available += n;
  return 0;
}
int v4_measure(const V4Sample *a, const V4Sample *b, V4Measurement *o) {
  if (!a || !b || !o || b->wall_ns < a->wall_ns || b->cpu_ns < a->cpu_ns ||
      b->read_bytes < a->read_bytes || b->write_accepted < a->write_accepted ||
      b->write_completed < a->write_completed || b->io_ops < a->io_ops)
    return -1;
  V4Measurement x = {0};
  x.delta.wall_ns = b->wall_ns - a->wall_ns;
  x.delta.cpu_ns = b->cpu_ns - a->cpu_ns;
  x.delta.read_bytes = b->read_bytes - a->read_bytes;
  x.delta.write_accepted = b->write_accepted - a->write_accepted;
  x.delta.write_completed = b->write_completed - a->write_completed;
  x.delta.io_ops = b->io_ops - a->io_ops;
  x.delta.cycles = b->cycles - a->cycles;
  x.cycle_wrap = b->cycles < a->cycles;
  *o = x;
  return 0;
}
