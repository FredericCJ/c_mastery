#include "replay_depth.h"
#include "stream.h"
#include <string.h>
#ifndef RPR_MUTATION
#define RPR_MUTATION 0
#endif
int rpr_input_frame(RprInput input, unsigned char out[18]) {
  unsigned char raw[16], wire[18];
  unsigned i;
  int r;
  if (!out)
    return RPR_INVALID;
  r = rpr_input_encode(input, raw);
  if (r)
    return r;
  for (i = 0; i < 2; ++i) {
    RfFrame f = {0};
    RfMessage m;
    f.length = 8;
    memcpy(f.bytes, raw + 8 * i, 8);
    if (rf_message_init(&m, i + 1, 1, &f))
      return RPR_STATE;
    memcpy(wire + 9 * i, m.wire, 9);
  }
  memcpy(out, wire, 18);
  return RPR_OK;
}
int rpr_input_unframe(const unsigned char *wire, size_t n, unsigned chunk,
                      RprInput *out) {
  RfParser p;
  unsigned char raw[16];
  size_t at = 0, count = 0;
  unsigned frames = 0;
  if (!wire || !out || !chunk || chunk > 32)
    return RPR_INVALID;
  if (rf_parser_init(&p, 1))
    return RPR_STATE;
  while (at < n) {
    RfFrame f;
    size_t used, take = n - at < chunk ? n - at : chunk;
    int r = rf_feed(&p, wire + at, take, &used, &f);
    at += used;
    if (r < 0)
      return RPR_CORRUPT;
    if (r == 1) {
      if ((f.length != 8 && RPR_MUTATION != 10) || f.length > 16 - count)
        return RPR_CORRUPT;
      memcpy(raw + count, f.bytes, f.length);
      count += f.length;
      ++frames;
      if (RPR_MUTATION == 9 && count == 16)
        break;
    }
  }
  if (rf_eof(&p) || count != 16 || (frames != 2 && RPR_MUTATION != 10))
    return RPR_CORRUPT;
  return rpr_input_decode(raw, out);
}
