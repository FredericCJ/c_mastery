#include "verifier.h"
#ifndef VV_MUTANT
#define VV_MUTANT 0
#endif
enum { NEXT, JUMP, COND, CALL, RETURN, SPLIT };
struct vg_rule {
  uint8_t width, need;
  int8_t delta;
  uint8_t flow;
};
/* One declared table supplies width, stack requirement/effect and edge kind.
 * CALL's arity-dependent requirement/effect is the one explicit override. */
static const struct vg_rule rules[24] = {
    {0, 0, 0, NEXT},  {5, 0, 1, NEXT},    {1, 0, 1, NEXT},  {1, 0, 1, NEXT},
    {1, 0, 1, NEXT},  {1, 2, -1, NEXT},   {1, 2, -1, NEXT}, {1, 2, -1, NEXT},
    {1, 2, -1, NEXT}, {1, 2, -1, NEXT},   {1, 2, -1, NEXT}, {2, 0, 1, NEXT},
    {2, 1, -1, NEXT}, {1, 1, -1, NEXT},   {3, 0, 0, JUMP},  {3, 1, -1, COND},
    {3, 0, 0, CALL},  {1, 1, -1, RETURN}, {1, 2, -1, NEXT}, {1, 1, 0, NEXT},
    {1, 1, 0, NEXT},  {1, 0, 0, NEXT},    {2, 1, 0, NEXT},  {5, 1, -1, SPLIT}};
static unsigned word(const uint8_t *p) {
  return (unsigned)p[0] | ((unsigned)p[1] << 8);
}
static void put(uint8_t *p, unsigned n) {
  p[0] = (uint8_t)n;
  p[1] = (uint8_t)(n >> 8);
}
static int fail(struct vv_report *r, unsigned phase, unsigned pc) {
  r->phase = phase;
  r->pc = pc;
  return (int)phase;
}
static unsigned successors(const struct cvm_program *p, unsigned pc, unsigned *next) {
  struct vg_rule q = rules[p->code[pc]];
  if (q.flow == RETURN)
    return 0;
  if (q.flow == JUMP) {
    next[0] = word(p->code + pc + 1);
    return 1;
  }
  if (q.flow == SPLIT) {
    next[0] = word(p->code + pc + 1);
    next[1] = word(p->code + pc + 3);
    return 2;
  }
  next[0] = pc + q.width;
  if (q.flow == COND) {
    next[1] = word(p->code + pc + 1);
    return 2;
  }
  return 1;
}
int vv_verify(const struct cvm_program *p, unsigned dialect, unsigned definite,
              struct vv_work *w, struct vv_report *r) {
  if (!p || !w || !r || dialect > 1 || definite > 1)
    return VV_ARGUMENT;
  *r = (struct vv_report){0};
  if (!p->count || p->count > CVM_FUNCTIONS || !p->length ||
      p->length > CVM_CODE || p->functions[0].arity)
    return fail(r, VV_LAYOUT, 0);
  for (unsigned i = 0; i < CVM_CODE; i++) {
    w->boundary[i] = 0;
    w->height[i] = -1;
    w->initialized[i] = 0;
    w->queued[i] = 0;
  }
  unsigned expected = 0;
  for (unsigned f = 0; f < p->count; f++) {
    const struct cvm_function *fn = p->functions + f;
    unsigned end = (unsigned)fn->entry + fn->length;
    if (fn->entry != expected || !fn->length || end > p->length ||
        fn->locals > CVM_LOCALS || fn->arity > fn->locals)
      return fail(r, VV_LAYOUT, fn->entry);
    for (unsigned pc = fn->entry; pc < end;) {
      unsigned op = p->code[pc];
      if (!op || op > VV_SPLIT || (op == VV_SPLIT && !dialect))
        return fail(r, VV_DECODE, pc);
      struct vg_rule q = rules[op];
      if (!q.width || q.width > end - pc)
        return fail(r, VV_DECODE, pc);
      if ((op == OP_LOAD || op == OP_STORE) && p->code[pc + 1] >= fn->locals)
        return fail(r, VV_DECODE, pc);
      if (op == OP_CALL &&
          (p->code[pc + 1] >= p->count ||
           p->code[pc + 2] != p->functions[p->code[pc + 1]].arity))
        return fail(r, VV_DECODE, pc);
      w->boundary[pc] = 1;
      r->decoded++;
      pc += q.width;
    }
    expected = end;
  }
  if (expected != p->length)
    return fail(r, VV_LAYOUT, expected);
  for (unsigned f = 0; f < p->count; f++) {
    const struct cvm_function *fn = p->functions + f;
    unsigned end = (unsigned)fn->entry + fn->length;
    for (unsigned pc = fn->entry; pc < end; pc += rules[p->code[pc]].width) {
      struct vg_rule q = rules[p->code[pc]];
      unsigned targets[2], n = 0;
      if (q.flow == JUMP || q.flow == COND) {
        targets[0] = word(p->code + pc + 1);
        n = 1;
      } else if (q.flow == SPLIT) {
        targets[0] = word(p->code + pc + 1);
        targets[1] = word(p->code + pc + 3);
        n = VV_MUTANT == 1 ? 1 : 2;
      }
      for (unsigned k = 0; k < n; k++)
        if (targets[k] < fn->entry || targets[k] >= end ||
            !w->boundary[targets[k]])
          return fail(r, VV_TARGET, pc);
    }
  }
  for (unsigned f = 0; f < p->count; f++) {
    const struct cvm_function *fn = p->functions + f;
    unsigned end = (unsigned)fn->entry + fn->length, head = 0, tail = 0,
             count = 0;
    w->height[fn->entry] = 0;
    w->initialized[fn->entry] = (uint8_t)((1u << fn->arity) - 1u);
    w->queue[tail++] = fn->entry;
    count++;
    w->queued[fn->entry] = 1;
    r->discovered++;
    if (count > r->max_queue)
      r->max_queue = count;
    while (count) {
      unsigned pc = w->queue[head];
      head = (head + 1) % CVM_CODE;
      count--;
      w->queued[pc] = 0;
      r->processed++;
      unsigned op = p->code[pc], need = rules[op].need;
      int delta = rules[op].delta, h = w->height[pc];
      uint8_t mask = w->initialized[pc];
      if (op == OP_CALL) {
        need = p->code[pc + 2];
        delta = 1 - (int)need;
      }
      if (h < (int)need || h + delta < 0 || h + delta > (int)CVM_STACK ||
          (op == OP_RET && h != 1))
        return fail(r, VV_HEIGHT, pc);
      if (definite && op == OP_LOAD && !(mask & (1u << p->code[pc + 1])))
        return fail(r, VV_INITIALIZED, pc);
      if (op == OP_STORE)
        mask |= (uint8_t)(1u << p->code[pc + 1]);
      unsigned next[2], n = successors(p, pc, next);
      for (unsigned k = 0; k < n; k++) {
        unsigned target = next[k];
        r->edges++;
        if (target < fn->entry || target >= end || !w->boundary[target])
          return fail(r, VV_TARGET, pc);
        int changed = 0;
        if (w->height[target] < 0) {
          w->height[target] = (int16_t)(h + delta);
          w->initialized[target] = mask;
          r->discovered++;
          changed = 1;
        } else {
          if (w->height[target] != h + delta && VV_MUTANT != 2)
            return fail(r, VV_HEIGHT, target);
          if (definite) {
            uint8_t joined = VV_MUTANT == 3
                                 ? (uint8_t)(w->initialized[target] | mask)
                                 : (uint8_t)(w->initialized[target] & mask);
            if (joined != w->initialized[target]) {
              w->initialized[target] = joined;
              r->local_changes++;
              changed = VV_MUTANT == 4 ? 0 : 1;
            }
          }
        }
        if (changed && !w->queued[target]) {
          if (count == CVM_CODE)
            return fail(r, VV_SPACE, target);
          w->queue[tail] = (uint16_t)target;
          tail = (tail + 1) % CVM_CODE;
          count++;
          w->queued[target] = 1;
          if (count > r->max_queue)
            r->max_queue = count;
        }
      }
    }
  }
  r->phase = VV_OK;
  r->pc = p->length;
  return VV_OK;
}
int vv_load(const uint8_t *b, size_t n, unsigned dialect, unsigned definite,
            struct vv_work *w, struct cvm_program *out, struct vv_report *r) {
  if (!out || !w || !r || dialect > 1 || definite > 1)
    return VV_ARGUMENT;
  struct cvm_program temp;
  if (cvm_decode(b, n, &temp) != CVM_OK) {
    *r = (struct vv_report){0};
    return fail(r, VV_LAYOUT, 0);
  }
  if (VV_MUTANT == 5)
    *out = temp;
  int status = vv_verify(&temp, dialect, definite, w, r);
  if (status == VV_OK)
    *out = temp;
  return status;
}
int vv_lower(const struct cvm_program *p, unsigned dialect, struct vv_work *w, struct cvm_program *out,
             struct vv_report *r) {
  if (!out)
    return VV_ARGUMENT;
  int status = vv_verify(p, dialect, 0, w, r);
  if (status != VV_OK)
    return status;
  unsigned pos = 0;
  for (unsigned pc = 0; pc < p->length; pc += rules[p->code[pc]].width) {
    w->relocation[pc] = (uint16_t)pos;
    pos += p->code[pc] == VV_SPLIT ? 6 : rules[p->code[pc]].width;
    if (pos > CVM_CODE)
      return fail(r, VV_SPACE, pc);
  }
  w->relocation[p->length] = (uint16_t)pos;
  struct cvm_program result = {0};
  result.count = p->count;
  result.length = (uint16_t)pos;
  for (unsigned f = 0; f < p->count; f++) {
    result.functions[f] = p->functions[f];
    unsigned start = p->functions[f].entry,
             end = start + p->functions[f].length;
    result.functions[f].entry = w->relocation[start];
    result.functions[f].length =
        (uint16_t)(w->relocation[end] - w->relocation[start]);
  }
  for (unsigned pc = 0; pc < p->length; pc += rules[p->code[pc]].width) {
    unsigned at = w->relocation[pc], op = p->code[pc];
    if (op == VV_SPLIT) {
      result.code[at] = OP_JFALSE;
      put(result.code + at + 1, w->relocation[word(p->code + pc + 3)]);
      result.code[at + 3] = OP_JMP;
      put(result.code + at + 4, VV_MUTANT == 6
                                    ? word(p->code + pc + 1)
                                    : w->relocation[word(p->code + pc + 1)]);
    } else {
      for (unsigned i = 0; i < rules[op].width; i++)
        result.code[at + i] = p->code[pc + i];
      if (op == OP_JMP || op == OP_JFALSE)
        put(result.code + at + 1, w->relocation[word(p->code + pc + 1)]);
    }
  }
  if (cvm_verify(&result) != CVM_OK)
    return fail(r, VV_TARGET, 0);
  *out = result;
  return VV_OK;
}
