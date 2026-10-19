#ifndef CMASTERY_COMPILER_SUCCESSOR_H
#define CMASTERY_COMPILER_SUCCESSOR_H
#include "verifier.h"
enum { VC_NODES = 4096, VC_PARSE = 260, VC_DEPTH = 256 };
typedef struct {
  unsigned kind, first, next;
  int32_t integer;
  char word[32];
} VcNode;
typedef struct {
  unsigned name, params, body, arity;
} VcDefinition;
typedef struct {
  unsigned node, stage, cursor, slot, a, b, used, head;
  int base, loop;
} VcFrame;
typedef struct {
  VcNode nodes[VC_NODES];
  unsigned parents[VC_PARSE], tails[VC_PARSE], count, parse_depth;
  VcDefinition definitions[CVM_FUNCTIONS];
  VcFrame frames[VC_DEPTH];
  unsigned next_patch[CVM_CODE], sp, used, peak, breaks, limit, folds;
  char names[CVM_LOCALS][32];
  int height, failed;
  CvmProgram staged;
  VvWork verification;
} VcWork;
typedef struct {
  unsigned expression_depth, fold_constants;
} VcOptions;
typedef struct {
  unsigned nodes, parser_peak, emitter_peak, code_bytes, local_peak, folded;
  VvReport verifier;
} VcReport;
/* Hosted original compiler. Explicit source<=4096 bytes; options depth1..256,
 * folding0/1. Default retained profile={64,0}. All controls/source/output are
 * disjoint live objects; serialized calls, caller-owned work. Work/report may
 * change on refusal; output program is unchanged. No recursive parser/emitter.
 * Actual vv_verify(definite initialization) and cvm_verify authorize output. */
CvmStatus vc_compile(const char *, size_t, const VcOptions *, VcWork *,
                     CvmProgram *, VcReport *);
#endif
