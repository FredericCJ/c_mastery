#include "execution.h"
#include <limits.h>
_Static_assert(UINT_MAX == UINT32_MAX,
               "VS profiles require 32-bit unsigned int arithmetic");
#ifndef VS_MUTANT
#define VS_MUTANT 0
#endif
static unsigned word(const uint8_t *p) {
  return (unsigned)p[0] | ((unsigned)p[1] << 8);
}
static int32_t signed_bits(uint32_t n) {
  return n <= INT32_MAX ? (int32_t)n : -1 - (int32_t)(UINT32_MAX - n);
}
static int32_t integer(const uint8_t *p) {
  uint32_t n = 0;
  for (unsigned i = 0; i < 4; i++)
    n |= (uint32_t)p[i] << (8 * i);
  return signed_bits(n);
}
int vs_arithmetic(unsigned mode, unsigned op, int32_t a, int32_t b,
                  int32_t *out) {
  if (!out || mode > VS_WRAP_V2 || op < OP_ADD || op > OP_DIV)
    return CVM_RANGE;
  int64_t n;
  if (op == OP_DIV) {
    if (!b)
      return CVM_RANGE;
    if (a == INT32_MIN && b == -1) {
      if (mode == VS_CHECKED_V1 || VS_MUTANT == 1)
        return CVM_RANGE;
      *out = INT32_MIN;
      return CVM_OK;
    }
    n = a / b;
  } else if (mode == VS_WRAP_V2 && VS_MUTANT != 1) {
    uint32_t x = (uint32_t)a, y = (uint32_t)b;
    *out = signed_bits(op == OP_ADD ? x + y : op == OP_SUB ? x - y : x * y);
    return CVM_OK;
  } else {
    n = op == OP_ADD   ? (int64_t)a + b
        : op == OP_SUB ? (int64_t)a - b
                       : (int64_t)a * b;
  }
  if (n < INT32_MIN || n > INT32_MAX)
    return CVM_RANGE;
  *out = (int32_t)n;
  return CVM_OK;
}
static void frame(struct cvm_frame *f, unsigned fn, unsigned ip, unsigned base) {
  *f = (struct cvm_frame){0};
  f->function = (uint8_t)fn;
  f->ip = (uint16_t)ip;
  f->base = (uint16_t)base;
}
int vs_start(struct vg_vs *s, const struct cvm_program *p, unsigned mode, uint32_t fuel,
             uint32_t gc, uint32_t host, uint32_t capabilities, struct vv_work *w) {
  if (!s || !p || !w || mode > VS_WRAP_V2)
    return CVM_RANGE;
  if (s->state == VS_RUNNING || (s->state == VS_PENDING && VS_MUTANT != 5) ||
      s->epoch == UINT32_MAX)
    return VS_STATE;
  if (!p->length || p->length > CVM_CODE || !p->count ||
      p->count > CVM_FUNCTIONS)
    return CVM_VERIFY;
  const uint8_t widths[25] = {0, 5, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2,
                              1, 3, 3, 3, 1, 1, 1, 1, 1, 2, 0, 3};
  struct cvm_program normal = *p;
  for (unsigned pc = 0; pc < p->length;) {
    unsigned op = p->code[pc];
    if (op > VS_TAIL || !widths[op] || widths[op] > p->length - pc)
      return CVM_VERIFY;
    if (op == VS_TAIL)
      normal.code[pc] = OP_CALL;
    pc += widths[op];
  }
  struct vv_report report;
  if (vv_verify(&normal, 0, 0, w, &report) != VV_OK)
    return CVM_VERIFY;
  for (unsigned fn = 0; fn < p->count; fn++) {
    unsigned end = p->functions[fn].entry + p->functions[fn].length;
    for (unsigned pc = p->functions[fn].entry; pc < end;
         pc += widths[p->code[pc]])
      if (p->code[pc] == VS_TAIL &&
          (pc + 3 >= end || p->code[pc + 3] != OP_RET ||
           (w->height[pc] >= 0 && w->height[pc] != p->code[pc + 2])))
        return CVM_VERIFY;
  }
  uint32_t epoch = s->epoch + 1;
  /* Avoid an entire automatic Vs compound-literal temporary at -O0.
   * cvm_init below assigns both pointer fields before they can be read. */
  unsigned char *storage = (unsigned char *)s;
  for (size_t i = 0; i < sizeof *s; i++)
    storage[i] = 0;
  s->epoch = epoch;
  s->image = *p;
  s->numeric_mode = mode;
  s->capabilities = capabilities;
  s->state = VS_RUNNING;
  s->invocation[0] = s->next_invocation = 1;
  cvm_init(&s->vm, fuel, gc, host);
  s->vm.depth = 1;
  frame(s->vm.frames, 0, p->functions[0].entry, 0);
  return VS_RUNNING;
}
static int fail(struct vg_vs *s, unsigned reason) {
  s->state = 1;
  s->reason = reason;
  return (int)reason;
}
static int push(struct cvm_cvm *v, struct cvm_value x) {
  if (v->sp >= CVM_STACK)
    return CVM_STACK_LIMIT;
  v->stack[v->sp++] = x;
  return CVM_OK;
}
int vs_take(struct vg_vs *s, struct vs_request *out) {
  if (!s || !out || s->state != VS_PENDING || (s->taken && VS_MUTANT != 6))
    return VS_STATE;
  *out = s->request;
  s->taken = 1;
  return VS_PENDING;
}
int vs_complete(struct vg_vs *s, uint32_t epoch, uint32_t serial, int failed,
                int32_t value) {
  if (!s || s->state != VS_PENDING || !s->taken ||
      (VS_MUTANT != 2 && (epoch != s->epoch || serial != s->serial)))
    return VS_STATE;
  if (failed)
    return fail(s, CVM_HOST_ERROR);
  int status = push(&s->vm, cvm_int(value));
  if (status != CVM_OK)
    return fail(s, (unsigned)status);
  s->state = VS_RUNNING;
  s->taken = 0;
  return VS_RUNNING;
}
int vs_step(struct vg_vs *s, struct cvm_value *out) {
  if (!s || !out)
    return CVM_RANGE;
  if (s->state != VS_RUNNING)
    return s->state == 1 ? (int)s->reason : VS_STATE;
  struct cvm_cvm *v = &s->vm;
  const struct cvm_program *p = &s->image;
  if (!v->fuel)
    return fail(s, CVM_FUEL);
  v->fuel--;
  struct cvm_frame *f = &v->frames[v->depth - 1];
  unsigned op = p->code[f->ip++], k;
  struct cvm_value a, b, r = {CVM_NIL, 0, 0};
  int status = CVM_OK;
  if (VS_MUTANT == 3)
    *out = r;
  switch (op) {
  case OP_INT:
    r = cvm_int(integer(p->code + f->ip));
    f->ip += 4;
    status = push(v, r);
    break;
  case OP_TRUE:
  case OP_FALSE:
    r.tag = CVM_BOOL;
    r.integer = op == OP_TRUE;
    status = push(v, r);
    break;
  case OP_NIL:
    status = push(v, r);
    break;
  case OP_LOAD:
    k = p->code[f->ip++];
    if (!f->initialized[k])
      return fail(s, CVM_TYPE);
    status = push(v, f->locals[k]);
    break;
  case OP_STORE:
    k = p->code[f->ip++];
    f->locals[k] = v->stack[--v->sp];
    f->initialized[k] = 1;
    break;
  case OP_POP:
    v->sp--;
    break;
  case OP_ADD:
  case OP_SUB:
  case OP_MUL:
  case OP_DIV:
  case OP_EQ:
  case OP_LT:
    b = v->stack[--v->sp];
    a = v->stack[--v->sp];
    if (op == OP_EQ) {
      r.tag = CVM_BOOL;
      r.integer = a.tag == b.tag && a.integer == b.integer &&
                  a.generation == b.generation;
    } else {
      if (a.tag != CVM_INT || b.tag != CVM_INT)
        return fail(s, CVM_TYPE);
      if (op == OP_LT) {
        r.tag = CVM_BOOL;
        r.integer = a.integer < b.integer;
      } else {
        r.tag = CVM_INT;
        status = vs_arithmetic(s->numeric_mode, op, a.integer, b.integer,
                               &r.integer);
      }
    }
    if (status == CVM_OK)
      status = push(v, r);
    break;
  case OP_JMP:
    f->ip = (uint16_t)word(p->code + f->ip);
    break;
  case OP_JFALSE:
    k = word(p->code + f->ip);
    f->ip += 2;
    a = v->stack[--v->sp];
    if (a.tag != CVM_BOOL)
      return fail(s, CVM_TYPE);
    if (!a.integer)
      f->ip = (uint16_t)k;
    break;
  case OP_CALL:
  case VS_TAIL: {
    unsigned fn = p->code[f->ip++], argc = p->code[f->ip++],
             base = v->sp - argc;
    int tail = op == VS_TAIL && VS_MUTANT != 4;
    if ((!tail && v->depth == CVM_FRAMES) || s->next_invocation == UINT32_MAX)
      return fail(s, CVM_FRAME_LIMIT);
    struct cvm_value args[CVM_LOCALS];
    for (k = 0; k < argc; k++)
      args[k] = v->stack[base + k];
    if (tail) {
      base = f->base;
    } else {
      f = &v->frames[v->depth++];
    }
    frame(f, fn, p->functions[fn].entry, base);
    s->invocation[v->depth - 1] = ++s->next_invocation;
    for (k = 0; k < argc; k++) {
      f->locals[k] = args[k];
      f->initialized[k] = 1;
    }
    v->sp = (uint16_t)base;
    break;
  }
  case OP_RET:
    r = v->stack[--v->sp];
    v->sp = f->base;
    v->depth--;
    if (!v->depth) {
      s->state = VS_DONE;
      *out = r;
      return VS_DONE;
    }
    status = push(v, r);
    break;
  case OP_PAIR:
    b = v->stack[--v->sp];
    a = v->stack[--v->sp];
    status = cvm_pair(v, a, b, &r);
    if (status == CVM_OK)
      status = push(v, r);
    break;
  case OP_FIRST:
  case OP_SECOND:
    a = v->stack[--v->sp];
    status = cvm_field(v, a, op == OP_SECOND, &r);
    if (status == CVM_OK)
      status = push(v, r);
    break;
  case OP_GC:
    status = cvm_collect(v);
    break;
  case OP_HOST:
    k = p->code[f->ip++];
    a = v->stack[--v->sp];
    if (a.tag != CVM_INT)
      return fail(s, CVM_TYPE);
    if (!v->host_budget)
      return fail(s, CVM_HOST_BUDGET);
    v->host_budget--;
    if (k >= 32 || !(s->capabilities & (UINT32_C(1) << k)) ||
        s->serial == UINT32_MAX)
      return fail(s, CVM_HOST_ERROR);
    s->request = (struct vs_request){s->epoch, ++s->serial, (uint8_t)k, a.integer};
    s->taken = 0;
    s->state = VS_PENDING;
    return VS_PENDING;
  default:
    return fail(s, CVM_VERIFY);
  }
  return status == CVM_OK ? VS_RUNNING : fail(s, (unsigned)status);
}
