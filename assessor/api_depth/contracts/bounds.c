#include "bounds.h"
#include <assert.h>
#include <limits.h>
_Static_assert(CHAR_BIT == 8, "wire-octet host requires eight-bit bytes");
enum ac_status ac_admit(struct ac_view v) {
  if (v.length > v.capacity || (v.capacity && !v.data)) return AC_INVALID;
  return AC_OK;
}
enum ac_status ac_timeout(int ms, struct ac_wait *out) {
  if (!out || ms > 60000) return AC_INVALID;
  struct ac_wait w = {ms < 0 ? AC_INFINITE : ms ? AC_FINITE : AC_POLL,
              ms > 0 ? (unsigned)ms : 0};
  *out = w; return AC_OK;
}
enum ac_status ac_deadline(unsigned now, struct ac_wait w, struct ac_deadline *out) {
  if (!out || now > 255 || w.tag > AC_INFINITE || w.tag < AC_POLL ||
      (w.tag != AC_FINITE && w.milliseconds) ||
      (w.tag == AC_FINITE && (!w.milliseconds || w.milliseconds > 127)))
    return AC_INVALID;
  *out = (struct ac_deadline){now, w.milliseconds, w.tag}; return AC_OK;
}
enum ac_status ac_expired(struct ac_deadline d, unsigned now, int *out) {
  struct ac_deadline admitted;
  if (!out || now > 255 || ac_deadline(d.start,(struct ac_wait){d.tag,d.duration},&admitted)!=AC_OK)
    return AC_INVALID;
  unsigned elapsed = (now - d.start) & 255u;
  if (elapsed > 127) return AC_INVALID;
#ifdef AC_MUTANT_WRAP
  *out = d.tag == AC_POLL || (d.tag == AC_FINITE && now >= d.start+d.duration);
#else
  *out = d.tag == AC_POLL || (d.tag == AC_FINITE && elapsed >= d.duration);
#endif
  return AC_OK;
}
enum ac_status ac_octets16(const uint_least16_t *p,size_t n,unsigned char *out,size_t cap) {
  if ((n && (!p || !out))) return AC_INVALID;
  if (n > cap / 2) return AC_FULL;
  /* uint_least16_t may be wider than16: reject before publishing any octet. */
#if UINT_LEAST16_MAX > 65535
  for (size_t i=0;i<n;++i) if ((uintmax_t)p[i] > UINTMAX_C(65535)) return AC_INVALID;
#endif
  for (size_t i=0;i<n;++i) {
    out[2*i]=(unsigned char)(p[i]&255u);
    out[2*i+1]=(unsigned char)(p[i]>>8);
  }
  return AC_OK;
}
enum ac_status ac_append_slice(struct api_buffer *b,struct ac_slice s) {
  if (!b || !s.owner || b->capacity>32 || b->length>b->capacity ||
      s.owner->capacity>32 || s.owner->length>s.owner->capacity ||
      s.offset>32 || s.count>32-s.offset) return AC_INVALID;
  if (s.count>b->capacity-b->length) return AC_FULL;
  if (s.count && s.owner==b) return AC_OVERLAP;
  for(size_t i=0;i<s.count;++i) b->data[b->length+i]=s.owner->data[s.offset+i];
  b->length+=s.count;return AC_OK;
}
enum f_status ac_append_allow(struct f_buffer *b,size_t offset,size_t n) {
  if (!b || offset>32 || n>32-offset) return F_INVALID;
  return f_append(b,n?b->data+offset:NULL,n);
}
static const int live_payload=17;
enum ac_status ac_lookup(unsigned key,struct ac_value *out) {
  if (!out) return AC_INVALID;
  if (key==2) return AC_MISSING;
  if (key>2) return AC_BACKEND;
#ifdef AC_MUTANT_SENTINEL
  if (!key) return AC_MISSING;
#endif
  *out=(struct ac_value){key?7:0,key?&live_payload:NULL};return AC_OK;
}
struct ac_result ac_lookup_tagged(unsigned key) {
  struct ac_result r={AC_INVALID,{0,NULL}};
  r.status=ac_lookup(key,&r.value);return r;
}
struct ac_progress ac_ascii_prefix(const unsigned char *p,size_t n,unsigned char *out,size_t cap) {
  struct ac_progress r={AC_OK,0,0};
  if ((n&&!p)||(cap&&!out)) { r.status=AC_INVALID;return r; }
  while (r.consumed<n) {
    if (p[r.consumed]>127) {r.status=AC_INVALID;break;}
    if (r.produced==cap) {r.status=AC_FULL;break;}
    out[r.produced++]=p[r.consumed++];
  }
  return r;
}
enum ac_status ac_copy_ascii(const unsigned char *p,size_t n,unsigned char *out,size_t cap,size_t *written,int init) {
  if (!written) return AC_INVALID;
  if (init) *written=0;
  if ((n&&!p)||(cap&&!out)) return AC_INVALID;
  if (n>cap) return AC_FULL;
  for(size_t i=0;i<n;++i) if(p[i]>127) return AC_INVALID;
  for(size_t i=0;i<n;++i) out[i]=p[i];
  *written=n;return AC_OK;
}
enum ac_status ac_checked_add(unsigned n,unsigned cap,unsigned *value,unsigned *eval) {
  if (!value||!eval) return AC_INVALID;
  ++*eval; /* caller guarantees counter headroom; required even with NDEBUG */
  if (*value>cap) return AC_INTERNAL;
  if (n>cap-*value) return AC_FULL;
#ifdef AC_MUTANT_ASSERT
  assert((*value+=n)<=cap);
#else
  *value+=n;
  assert(*value<=cap);
#endif
  return AC_OK;
}
enum ac_status ac_admit_wire(const unsigned char *p,size_t n,unsigned *out) {
  if(!p||!out||n!=3) return AC_INVALID;
  if(p[1]>32||p[0]>p[1]) return AC_INVALID;
  unsigned value=p[0],evaluations=0;
  enum ac_status r=ac_checked_add(p[2],p[1],&value,&evaluations);
  if(r==AC_OK) *out=value;
  return r;
}
