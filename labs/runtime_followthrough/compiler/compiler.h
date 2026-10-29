#ifndef CMASTERY_COMPILER_SUCCESSOR_H
#define CMASTERY_COMPILER_SUCCESSOR_H
#include "verifier.h"

enum
{
    VC_NODES = 4096,
    VC_PARSE = 260,
    VC_DEPTH = 256
};

struct vc_node
{
    unsigned kind, first, next;
    int32_t integer;
    char word[32];
};

struct vc_definition
{
    unsigned name, params, body, arity;
};

struct vc_frame
{
    unsigned node, stage, cursor, slot, a, b, used, head;
    int base, loop;
};

struct vc_work
{
    struct vc_node nodes[VC_NODES];
    unsigned parents[VC_PARSE], tails[VC_PARSE], count, parse_depth;
    struct vc_definition definitions[CVM_FUNCTIONS];
    struct vc_frame frames[VC_DEPTH];
    unsigned next_patch[CVM_CODE], sp, used, peak, breaks, limit, folds;
    char names[CVM_LOCALS][32];
    int height, failed;
    struct cvm_program staged;
    struct vv_work verification;
};

struct vc_options
{
    unsigned expression_depth, fold_constants;
};

struct vc_report
{
    unsigned nodes, parser_peak, emitter_peak, code_bytes, local_peak, folded;
    struct vv_report verifier;
};
/* Hosted original compiler. Explicit source<=4096 bytes; options depth1..256,
 * folding0/1. Default retained profile={64,0}. All controls/source/output are
 * disjoint live objects; serialized calls, caller-owned work. Work/report may
 * change on refusal; output program is unchanged. No recursive parser/emitter.
 * Actual vv_verify(definite initialization) and cvm_verify authorize output. */
enum cvm_status vc_compile(const char *, size_t, const struct vc_options *,
                           struct vc_work *, struct cvm_program *,
                           struct vc_report *);
#endif
