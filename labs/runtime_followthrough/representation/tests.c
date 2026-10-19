#include "representation.h"
#ifdef RV_STANDALONE
int printf(const char *, ...);
int puts(const char *);
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
int memcmp(const void *, const void *, size_t);
#else
#include <stdio.h>
#include <string.h>
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("RV_CHECK %u %s\n", (unsigned)__LINE__, #x);                      \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static const uint8_t literal[] = {67, 86, 77, 49, 1, 0, 0, 6, 0,
                                  0,  0,  1,  42, 0, 0, 0, 17};
static CvmProgram simple(void) {
  CvmProgram p = {0};
  p.count = 1;
  p.length = 6;
  p.functions[0].length = 6;
  p.code[0] = OP_INT;
  p.code[1] = 42;
  p.code[5] = OP_RET;
  return p;
}
static int values(void) {
  CvmValue old[] = {
      {CVM_INT, 0, 0}, {CVM_BOOL, 0, 0}, {CVM_NIL, 0, 0}, {CVM_REF, 0, 1}};
  RvValue v[4], sentinel = {RV_INT, 91, 0}, out = sentinel;
  int eq = 93;
  for (unsigned i = 0; i < 4; i++)
    CHECK(rv_import(old[i], &v[i]) == RV_OK);
  for (unsigned i = 0; i < 4; i++)
    for (unsigned j = 0; j < 4; j++) {
      CHECK(rv_equal(v[i], v[j], &eq) == RV_OK);
      CHECK(eq == (i == j));
    }
  CHECK(rv_arithmetic(RV_ADD, v[1], v[0], &out) == RV_TYPE &&
        out.payload == 91);
  CHECK(rv_equal(v[3], (RvValue){RV_REF, 0, 2}, &eq) == RV_OK && !eq);
  CHECK(rv_equal((RvValue){RV_NIL, 123, 99}, v[2], &eq) == RV_OK && eq);
  CHECK(rv_equal((RvValue){RV_INT, 0, 99}, v[0], &eq) == RV_OK && eq);
  CHECK(rv_import((CvmValue){CVM_REF, 16, 1}, &out) == RV_TYPE);
  CHECK(rv_import((CvmValue){CVM_REF, 0, 0}, &out) == RV_TYPE);
  CHECK(rv_import((CvmValue){CVM_BOOL, 2, 0}, &out) == RV_TYPE);
  CHECK(rv_import((CvmValue){(CvmTag)99, 0, 0}, &out) == RV_TYPE);
  CHECK(rv_arithmetic(RV_ADD, (RvValue){RV_INT, 32767, 0},
                      (RvValue){RV_Q16, 32768, 0}, &out) == RV_OK &&
        out.tag == RV_Q16 && out.payload == 2147450880);
  CHECK(rv_arithmetic(RV_ADD, (RvValue){RV_INT, 32768, 0},
                      (RvValue){RV_Q16, -65536, 0}, &out) == RV_RANGE &&
        out.payload == 2147450880);
  CHECK(rv_arithmetic(RV_MUL, (RvValue){RV_Q16, -1, 0},
                      (RvValue){RV_Q16, 32768, 0}, &out) == RV_OK &&
        out.payload == 0);
  CHECK(rv_arithmetic(RV_DIV, (RvValue){RV_Q16, -65536, 0},
                      (RvValue){RV_INT, 3, 0}, &out) == RV_OK &&
        out.payload == -21845);
  CHECK(rv_arithmetic(RV_DIV, (RvValue){RV_INT, INT32_MIN, 0},
                      (RvValue){RV_INT, -1, 0}, &out) == RV_RANGE &&
        out.payload == -21845);
  CHECK(rv_arithmetic(RV_MUL, (RvValue){RV_Q16, INT32_MIN, 0},
                      (RvValue){RV_Q16, INT32_MIN, 0}, &out) == RV_RANGE);
  CHECK(rv_equal((RvValue){RV_INT, 1, 0}, (RvValue){RV_Q16, 65536, 0}, &eq) ==
            RV_OK &&
        !eq);
  CHECK(rv_arithmetic(99, v[0], v[0], &out) == RV_ARGUMENT);
  CHECK(rv_equal(v[0], (RvValue){99, 0, 0}, &eq) == RV_TYPE);
  puts("RV_VALUES_PASS tags_strict_mixed_Q16_explicit");
  return 0;
}
static int raw(void) {
  CvmProgram p = simple(), out = {0}, before;
  uint8_t b[1100], saved[1100];
  size_t n = 77;
  memset(b, 91, sizeof b);
  CHECK(cvm_encode(&p, b, sizeof b, &n) == CVM_OK);
  CHECK(n == sizeof literal && !memcmp(b, literal, n));
  memset(&out, 0xa5, sizeof out);
  memcpy(&before, &out, sizeof out);
  for (size_t i = 0; i < n; i++) {
    CHECK(cvm_decode(b, i, &out) == CVM_FORMAT);
    CHECK(!memcmp(&out, &before, sizeof out));
  }
  for (size_t i = 0; i < n; i++) {
    size_t written = 79;
    memcpy(saved, b, sizeof b);
    CHECK(cvm_encode(&p, b, i, &written) == CVM_RANGE);
    CHECK(written == 79 && !memcmp(b, saved, sizeof b));
  }
  CHECK(cvm_decode(b, n, &out) == CVM_OK && out.code[1] == 42);
  p.code[1] = p.code[2] = p.code[3] = 0;
  p.code[4] = 128;
  CHECK(cvm_encode(&p, b, sizeof b, &n) == CVM_OK && b[12] == 0 &&
        b[15] == 128);
  p.code[0] = 255;
  CHECK(cvm_encode(&p, b, sizeof b, &n) == CVM_OK);
  CHECK(cvm_decode(b, n, &out) == CVM_OK && out.code[0] == 255);
  /* independent single-defect containers, each failing raw structural phase */
  static const unsigned index[] = {0, 4, 5, 7, 9, 10};
  static const uint8_t replacement[] = {0, 9, 1, 7, 1, 9};
  unsigned cases = 0;
  for (unsigned i = 0; i < 6; i++) {
    memcpy(b, literal, sizeof literal);
    b[index[i]] = replacement[i];
    memcpy(&before, &out, sizeof out);
    CHECK(cvm_decode(b, sizeof literal, &out) == CVM_FORMAT &&
          !memcmp(&out, &before, sizeof out));
    cases++;
  }
  memcpy(b, literal, sizeof literal);
  CHECK(cvm_decode(b, SIZE_MAX, &out) == CVM_FORMAT);
  CvmProgram two = {0};
  two.count = 2;
  two.length = 2;
  two.code[0] = 255;
  two.code[1] = 254;
  two.functions[0].length = 1;
  two.functions[1] = (CvmFunction){1, 1, 1, 1};
  CHECK(cvm_encode(&two, b, sizeof b, &n) == CVM_OK &&
        cvm_decode(b, n, &out) == CVM_OK);
  b[11] = 2;
  CHECK(cvm_decode(b, n, &out) == CVM_FORMAT); /* gap */
  b[11] = 0;
  CHECK(cvm_decode(b, n, &out) == CVM_FORMAT); /* overlap */
  printf("RV_RAW_PASS truncations=17 malformed=%u width_size=%u width_int=%u\n",
         cases + 3, (unsigned)sizeof(size_t), (unsigned)sizeof(int));
  return 0;
}
static int envelope(void) {
  RvImage in = {0}, out, before;
  uint8_t b[RV_IMAGE_MAX] = {0}, saved[RV_IMAGE_MAX];
  size_t n = 91;
  in.program = simple();
  in.name_length = 3;
  memcpy(in.name, "abc", 3);
  in.has_digest = 1;
  for (unsigned i = 0; i < 32; i++)
    in.digest[i] = (uint8_t)i;
  CHECK(rv_encode(&in, b, sizeof b, &n) == RV_OK && n == 69);
  static const uint8_t prefix[] = {82, 86, 69, 50, 1, 0, 17, 0, 2};
  CHECK(!memcmp(b, prefix, 9) && !memcmp(b + 9, literal, 17));
  CHECK(b[26] == 1 && b[27] == 0 && b[28] == 32 && b[29] == 0 && b[62] == 2 &&
        b[64] == 3);
  CHECK(rv_decode(b, n, &out) == RV_OK && out.program.code[1] == 42 &&
        out.name_length == 3 && out.has_digest &&
        !memcmp(out.digest, in.digest, 32));
  memcpy(&before, &out, sizeof out);
  for (size_t i = 0; i < n; i++) {
    CHECK(rv_decode(b, i, &out) != RV_OK);
    CHECK(!memcmp(&out, &before, sizeof out));
  }
  for (size_t i = 0; i < n; i++) {
    size_t z = 93;
    memcpy(saved, b, sizeof b);
    CHECK(rv_encode(&in, b, i, &z) == RV_RANGE && z == 93 &&
          !memcmp(saved, b, sizeof b));
  }
  CvmProgram p;
  CHECK(cvm_decode(b, n, &p) == CVM_FORMAT);
  b[8] = 3;
  b[n] = 99;
  b[n + 1] = 0;
  b[n + 2] = 2;
  b[n + 3] = 0;
  b[n + 4] = 7;
  b[n + 5] = 8;
  CHECK(rv_decode(b, n + 6, &out) == RV_OK && out.skipped == 1);
  memcpy(&before, &out, sizeof out);
  b[n + 1] = 1;
  CHECK(rv_decode(b, n + 6, &out) == RV_REQUIRED &&
        !memcmp(&before, &out, sizeof out));
  b[n + 1] = 0;
  b[n + 2] = 3;
  CHECK(rv_decode(b, n + 6, &out) == RV_FORMAT &&
        !memcmp(&before, &out, sizeof out));
  b[8] = 2;
  b[n] = 0;
  CHECK(rv_decode(b, n + 1, &out) == RV_FORMAT &&
        !memcmp(&before, &out, sizeof out));
  b[4] = 2;
  CHECK(rv_decode(b, n, &out) == RV_VERSION);
  b[4] = 1;
  b[5] = 1;
  CHECK(rv_decode(b, n, &out) == RV_FORMAT);
  b[5] = 0;
  b[8] = 5;
  CHECK(rv_decode(b, n, &out) == RV_FORMAT);
  b[8] = 2;
  b[27] = 2;
  CHECK(rv_decode(b, n, &out) == RV_FORMAT);
  b[27] = 0;
  b[28] = 31;
  CHECK(rv_decode(b, n, &out) == RV_FORMAT);
  b[28] = 32;
  b[66] = 0;
  CHECK(rv_decode(b, n, &out) == RV_FORMAT);
  b[66] = 'a';
  b[62] = 1;
  CHECK(rv_decode(b, n, &out) == RV_FORMAT);
  b[62] = 2;
  CHECK(rv_decode(b, SIZE_MAX, &out) == RV_FORMAT);
  CHECK(rv_decode(b, 0, &out) == RV_FORMAT);
  /* All optional unknown lengths admitted up to64, rejected at65; critical
   * flips independently. */
  in.has_digest = 0;
  in.name_length = 0;
  CHECK(rv_encode(&in, b, sizeof b, &n) == RV_OK && n == 26);
  b[8] = 1;
  for (unsigned len = 0; len <= 65; len++) {
    b[n] = 200;
    b[n + 1] = 0;
    b[n + 2] = (uint8_t)len;
    b[n + 3] = 0;
    memset(b + n + 4, 11, len);
    CHECK(rv_decode(b, n + 4 + len, &out) == (len <= 64 ? RV_OK : RV_FORMAT));
    b[n + 1] = 1;
    CHECK(rv_decode(b, n + 4 + len, &out) ==
          (len <= 64 ? RV_REQUIRED : RV_FORMAT));
  }
  puts("RV_ENVELOPE_PASS truncations=69 unknown_length_cases=132 "
       "old_reader_rejects");
  return 0;
}
int main(void) {
  CHECK(!values());
  CHECK(!raw());
  CHECK(!envelope());
  puts("RV_PASS representation_codec_changed_conditions");
  return 0;
}
