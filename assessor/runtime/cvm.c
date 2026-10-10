#include "cvm.h"
#include <limits.h>
static CvmValue nil(void) {
  CvmValue v = {CVM_NIL, 0, 0};
  return v;
}
CvmValue cvm_int(int32_t n) {
  CvmValue v = {CVM_INT, n, 0};
  return v;
}
static CvmValue boolean(int n) {
  CvmValue v = {CVM_BOOL, n != 0, 0};
  return v;
}
static uint16_t u16(const uint8_t *p) {
  return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}
static void put16(uint8_t *p, uint16_t n) {
  p[0] = (uint8_t)n;
  p[1] = (uint8_t)(n >> 8);
}
static int32_t i32(const uint8_t *p) {
  uint32_t n = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) |
               ((uint32_t)p[3] << 24);
  return n <= INT32_MAX ? (int32_t)n
                        : (int32_t)((int64_t)n - INT64_C(4294967296));
}
const char *cvm_status_name(CvmStatus s) {
  static const char *const names[] = {
      "OK",          "FORMAT",      "VERIFY", "TYPE",       "RANGE",
      "STACK_LIMIT", "FRAME_LIMIT", "FUEL",   "HEAP_LIMIT", "GC_BUDGET",
      "HOST_BUDGET", "HOST_ERROR",  "STALE",  "SYNTAX",     "NOT_IMPLEMENTED"};
  return (unsigned)s < sizeof names / sizeof names[0] ? names[s] : "UNKNOWN";
}
void cvm_init(Cvm *v, uint32_t fuel, uint32_t gc, uint32_t host) {
  unsigned i, j;
  v->sp = 0;
  v->depth = 0;
  v->root_count = 0;
  v->arena = 0;
  v->fuel = fuel;
  v->gc_budget = gc;
  v->host_budget = host;
  v->allocations = 0;
  v->collections = 0;
  v->host = 0;
  v->host_context = 0;
  for (i = 0; i < CVM_HEAP; i++) {
    v->heap[i].used = 0;
    v->heap[i].marked = 0;
    v->heap[i].generation = 0;
    v->heap[i].left = nil();
    v->heap[i].right = nil();
  }
  for (i = 0; i < CVM_FRAMES; i++)
    for (j = 0; j < CVM_LOCALS; j++)
      v->frames[i].initialized[j] = 0;
}
static unsigned width(unsigned op) {
  switch (op) {
  case OP_INT:
    return 5;
  case OP_LOAD:
  case OP_STORE:
  case OP_HOST:
    return 2;
  case OP_JMP:
  case OP_JFALSE:
  case OP_CALL:
    return 3;
  default:
    return op >= OP_TRUE && op <= OP_GC ? 1 : 0;
  }
}
CvmStatus cvm_verify(const CvmProgram *p) {
  uint8_t boundary[CVM_CODE];
  int16_t height[CVM_CODE];
  uint16_t queue[CVM_CODE];
  unsigned f, i, expected = 0;
  if (!p || !p->count || p->count > CVM_FUNCTIONS || !p->length ||
      p->length > CVM_CODE || p->functions[0].arity)
    return CVM_VERIFY;
  for (i = 0; i < CVM_CODE; i++) {
    boundary[i] = 0;
    height[i] = -1;
  }
  for (f = 0; f < p->count; f++) {
    const CvmFunction *fn = &p->functions[f];
    unsigned end = (unsigned)fn->entry + fn->length, pc = fn->entry;
    if (pc != expected || !fn->length || end > p->length ||
        fn->arity > fn->locals || fn->locals > CVM_LOCALS)
      return CVM_VERIFY;
    while (pc < end) {
      unsigned op = p->code[pc], w = width(op);
      if (!w || w > end - pc)
        return CVM_VERIFY;
      boundary[pc] = 1;
      if ((op == OP_LOAD || op == OP_STORE) && p->code[pc + 1] >= fn->locals)
        return CVM_VERIFY;
      if (op == OP_CALL &&
          (p->code[pc + 1] >= p->count ||
           p->code[pc + 2] != p->functions[p->code[pc + 1]].arity))
        return CVM_VERIFY;
      pc += w;
    }
    expected = end;
  }
  if (expected != p->length)
    return CVM_VERIFY;
  for (f = 0; f < p->count; f++) {
    const CvmFunction *fn = &p->functions[f];
    unsigned end = (unsigned)fn->entry + fn->length, head = 0, tail = 0, pc;
    for (pc = fn->entry; pc < end; pc += width(p->code[pc]))
      if (p->code[pc] == OP_JMP || p->code[pc] == OP_JFALSE) {
        unsigned t = u16(&p->code[pc + 1]);
        if (t < fn->entry || t >= end || !boundary[t])
          return CVM_VERIFY;
      }
    queue[tail++] = fn->entry;
    height[fn->entry] = 0;
    while (head < tail) {
      unsigned op, w, next[2], nnext = 1, k, need = 0;
      int h, delta = 0;
      pc = queue[head++];
      op = p->code[pc];
      w = width(op);
      h = height[pc];
      next[0] = pc + w;
      switch (op) {
      case OP_INT:
      case OP_TRUE:
      case OP_FALSE:
      case OP_NIL:
      case OP_LOAD:
        delta = 1;
        break;
      case OP_ADD:
      case OP_SUB:
      case OP_MUL:
      case OP_DIV:
      case OP_EQ:
      case OP_LT:
      case OP_PAIR:
        need = 2;
        delta = -1;
        break;
      case OP_STORE:
      case OP_POP:
        need = 1;
        delta = -1;
        break;
      case OP_FIRST:
      case OP_SECOND:
      case OP_HOST:
        need = 1;
        break;
      case OP_JFALSE:
        need = 1;
        delta = -1;
        next[1] = u16(&p->code[pc + 1]);
        nnext = 2;
        break;
      case OP_JMP:
        next[0] = u16(&p->code[pc + 1]);
        break;
      case OP_CALL:
        need = p->code[pc + 2];
        delta = 1 - (int)need;
        break;
      case OP_RET:
        if (h != 1)
          return CVM_VERIFY;
        nnext = 0;
        break;
      default:
        break;
      }
      if (h < (int)need || h + delta < 0 || h + delta > (int)CVM_STACK)
        return CVM_VERIFY;
      for (k = 0; k < nnext; k++) {
        unsigned t = next[k];
        if (t < fn->entry || t >= end || !boundary[t])
          return CVM_VERIFY;
        if (height[t] >= 0) {
          if (height[t] != h + delta)
            return CVM_VERIFY;
        } else {
          height[t] = (int16_t)(h + delta);
          queue[tail++] = (uint16_t)t;
        }
      }
    }
  }
  return CVM_OK;
}
/* Container structure is an earlier boundary than instruction verification. */
static CvmStatus layout(const CvmProgram *p) {
  unsigned expected = 0;
  if (!p || !p->count || p->count > CVM_FUNCTIONS || !p->length ||
      p->length > CVM_CODE)
    return CVM_FORMAT;
  for (unsigned f = 0; f < p->count; ++f) {
    const CvmFunction *fn = &p->functions[f];
    unsigned end = (unsigned)fn->entry + fn->length;
    if (fn->entry != expected || !fn->length || end > p->length ||
        fn->arity > fn->locals || fn->locals > CVM_LOCALS)
      return CVM_FORMAT;
    expected = end;
  }
  return expected == p->length ? CVM_OK : CVM_FORMAT;
}
CvmStatus cvm_decode(const uint8_t *b, size_t n, CvmProgram *out) {
  CvmProgram p = {0};
  size_t header, i;
  unsigned f;
  CvmStatus s;
  if (!b || !out || n < 5 || b[0] != 'C' || b[1] != 'V' || b[2] != 'M' ||
      b[3] != '1' || !b[4] || b[4] > CVM_FUNCTIONS)
    return CVM_FORMAT;
  p.count = b[4];
  header = 5u + 6u * p.count;
  if (n < header || n - header > CVM_CODE)
    return CVM_FORMAT;
  p.length = (uint16_t)(n - header);
  for (f = 0; f < p.count; f++) {
    const uint8_t *d = b + 5u + 6u * f;
    p.functions[f].entry = u16(d);
    p.functions[f].length = u16(d + 2);
    p.functions[f].arity = d[4];
    p.functions[f].locals = d[5];
  }
  for (i = 0; i < p.length; i++)
    p.code[i] = b[header + i];
  s = layout(&p);
  if (s == CVM_OK)
    *out = p;
  return s;
}
CvmStatus cvm_encode(const CvmProgram *p, uint8_t *b, size_t capacity,
                     size_t *written) {
  size_t n, i;
  unsigned f;
  CvmStatus s = layout(p);
  if (s != CVM_OK)
    return s;
  n = 5u + 6u * p->count + p->length;
  if (!b || !written || capacity < n)
    return CVM_RANGE;
  b[0] = 'C';
  b[1] = 'V';
  b[2] = 'M';
  b[3] = '1';
  b[4] = p->count;
  for (f = 0; f < p->count; f++) {
    uint8_t *d = b + 5u + 6u * f;
    put16(d, p->functions[f].entry);
    put16(d + 2, p->functions[f].length);
    d[4] = p->functions[f].arity;
    d[5] = p->functions[f].locals;
  }
  for (i = 0; i < p->length; i++)
    b[5u + 6u * p->count + i] = p->code[i];
  *written = n;
  return CVM_OK;
}
CvmStatus cvm_load(const uint8_t *b, size_t n, CvmProgram *out) {
  CvmProgram p;
  if (!out)
    return CVM_FORMAT;
  CvmStatus s = cvm_decode(b, n, &p);
  if (s == CVM_OK)
    s = cvm_verify(&p);
  if (s == CVM_OK)
    *out = p;
  return s;
}
CvmStatus cvm_save(const CvmProgram *p, uint8_t *b, size_t capacity,
                   size_t *written) {
  CvmStatus s = cvm_verify(p);
  return s == CVM_OK ? cvm_encode(p, b, capacity, written) : s;
}
static CvmStatus live(Cvm *v, CvmValue r) {
  if (r.tag != CVM_REF)
    return CVM_TYPE;
  if (r.integer < 0 || (uint32_t)r.integer >= CVM_HEAP ||
      !v->heap[r.integer].used || v->heap[r.integer].generation != r.generation)
    return CVM_STALE;
  return CVM_OK;
}
CvmStatus cvm_field(Cvm *v, CvmValue r, int right, CvmValue *out) {
  CvmStatus s = live(v, r);
  if (s == CVM_OK)
    *out = right ? v->heap[r.integer].right : v->heap[r.integer].left;
  return s;
}
CvmStatus cvm_root_push(Cvm *v, CvmValue x) {
  if (v->root_count >= CVM_ROOTS)
    return CVM_STACK_LIMIT;
  if (x.tag == CVM_REF) {
    CvmStatus s = live(v, x);
    if (s != CVM_OK)
      return s;
  }
  v->roots[v->root_count++] = x;
  return CVM_OK;
}
void cvm_root_pop(Cvm *v) {
  if (v->root_count)
    v->root_count--;
}
static CvmStatus mark(Cvm *v, CvmValue x, uint8_t *work, unsigned *n) {
  CvmStatus s;
  if (!v->gc_budget)
    return CVM_GC_BUDGET;
  v->gc_budget--;
  if (x.tag != CVM_REF)
    return CVM_OK;
  s = live(v, x);
  if (s != CVM_OK)
    return s;
  if (!v->heap[x.integer].marked) {
    v->heap[x.integer].marked = 1;
    work[(*n)++] = (uint8_t)x.integer;
  }
  return CVM_OK;
}
CvmStatus cvm_collect(Cvm *v) {
  uint8_t work[CVM_HEAP];
  unsigned n = 0, i, j;
  CvmStatus s;
  if (v->arena)
    return CVM_HEAP_LIMIT;
  /* Sweep starts only after complete marking. Budget exhaustion never frees a
   * live object. */
  for (i = 0; i < CVM_HEAP; i++)
    v->heap[i].marked = 0;
#ifndef CVM_MUTATE_SKIP_ROOTS
  for (i = 0; i < v->root_count; i++) {
    s = mark(v, v->roots[i], work, &n);
    if (s != CVM_OK)
      return s;
  }
#endif
  for (i = 0; i < v->sp; i++) {
    s = mark(v, v->stack[i], work, &n);
    if (s != CVM_OK)
      return s;
  }
  for (i = 0; i < v->depth; i++)
    for (j = 0; j < CVM_LOCALS; j++)
      if (v->frames[i].initialized[j]) {
        s = mark(v, v->frames[i].locals[j], work, &n);
        if (s != CVM_OK)
          return s;
      }
  while (n) {
    CvmObject *o = &v->heap[work[--n]];
    s = mark(v, o->left, work, &n);
    if (s != CVM_OK)
      return s;
    s = mark(v, o->right, work, &n);
    if (s != CVM_OK)
      return s;
  }
  if (v->gc_budget < CVM_HEAP)
    return CVM_GC_BUDGET;
  v->gc_budget -= CVM_HEAP;
  for (i = 0; i < CVM_HEAP; i++)
    if (v->heap[i].used && !v->heap[i].marked)
      v->heap[i].used = 0;
  v->collections++;
  return CVM_OK;
}
CvmStatus cvm_pair(Cvm *v, CvmValue a, CvmValue b, CvmValue *out) {
  unsigned i, pass;
  CvmStatus s;
  if (a.tag == CVM_REF && (s = live(v, a)) != CVM_OK)
    return s;
  if (b.tag == CVM_REF && (s = live(v, b)) != CVM_OK)
    return s;
  for (pass = 0; pass < 2; pass++) {
    for (i = 0; i < CVM_HEAP; i++)
      if (!v->heap[i].used && v->heap[i].generation != UINT32_MAX) {
        CvmObject *o = &v->heap[i];
        o->generation++;
        o->used = 1;
        o->marked = 0;
        o->left = a;
        o->right = b;
        out->tag = CVM_REF;
        out->integer = (int32_t)i;
        out->generation = o->generation;
        v->allocations++;
        return CVM_OK;
      }
    if (pass || v->arena)
      return CVM_HEAP_LIMIT;
    /* C temporaries a/b must survive the allocation-triggered collection. */
    s = cvm_root_push(v, a);
    if (s != CVM_OK)
      return s;
    s = cvm_root_push(v, b);
    if (s != CVM_OK) {
      cvm_root_pop(v);
      return s;
    }
    s = cvm_collect(v);
    cvm_root_pop(v);
    cvm_root_pop(v);
    if (s != CVM_OK)
      return s;
  }
  return CVM_HEAP_LIMIT;
}
static CvmStatus push(Cvm *v, CvmValue x) {
  if (v->sp >= CVM_STACK)
    return CVM_STACK_LIMIT;
  v->stack[v->sp++] = x;
  return CVM_OK;
}
static void frame(CvmFrame *f, uint8_t fn, uint16_t ip, uint16_t base) {
  unsigned i;
  f->function = fn;
  f->ip = ip;
  f->base = base;
  for (i = 0; i < CVM_LOCALS; i++)
    f->initialized[i] = 0;
}
CvmStatus cvm_run(Cvm *v, const CvmProgram *p, CvmValue *out) {
  CvmStatus s = cvm_verify(p);
  if (s != CVM_OK)
    return s;
  if (!v || !out)
    return CVM_RANGE;
  v->sp = 0;
  v->depth = 1;
  frame(&v->frames[0], 0, p->functions[0].entry, 0);
  while (v->depth) {
    CvmFrame *f = &v->frames[v->depth - 1];
    unsigned op = p->code[f->ip++];
    CvmValue a, b, r;
    int64_t n = 0;
    unsigned k;
#ifndef CVM_MUTATE_NO_FUEL
    if (!v->fuel)
      return CVM_FUEL;
    v->fuel--;
#endif
    switch (op) {
    case OP_INT:
      r = cvm_int(i32(&p->code[f->ip]));
      f->ip += 4;
      s = push(v, r);
      break;
    case OP_TRUE:
    case OP_FALSE:
      s = push(v, boolean(op == OP_TRUE));
      break;
    case OP_NIL:
      s = push(v, nil());
      break;
    case OP_LOAD:
      k = p->code[f->ip++];
      if (!f->initialized[k])
        return CVM_TYPE;
      s = push(v, f->locals[k]);
      break;
    case OP_STORE:
      k = p->code[f->ip++];
      f->locals[k] = v->stack[--v->sp];
      f->initialized[k] = 1;
      s = CVM_OK;
      break;
    case OP_POP:
      v->sp--;
      s = CVM_OK;
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
        r = boolean(a.tag == b.tag && a.integer == b.integer &&
                    a.generation == b.generation);
        s = push(v, r);
        break;
      }
      if (a.tag != CVM_INT || b.tag != CVM_INT)
        return CVM_TYPE;
      if (op == OP_LT) {
        s = push(v, boolean(a.integer < b.integer));
        break;
      }
      if (op == OP_DIV) {
        if (!b.integer || (a.integer == INT32_MIN && b.integer == -1))
          return CVM_RANGE;
        n = a.integer / b.integer;
      } else if (op == OP_ADD)
        n = (int64_t)a.integer + b.integer;
      else if (op == OP_SUB)
        n = (int64_t)a.integer - b.integer;
      else
        n = (int64_t)a.integer * b.integer;
      if (n < INT32_MIN || n > INT32_MAX)
        return CVM_RANGE;
      s = push(v, cvm_int((int32_t)n));
      break;
    case OP_JMP:
      f->ip = u16(&p->code[f->ip]);
      s = CVM_OK;
      break;
    case OP_JFALSE:
      k = u16(&p->code[f->ip]);
      f->ip += 2;
      a = v->stack[--v->sp];
      if (a.tag != CVM_BOOL)
        return CVM_TYPE;
      if (!a.integer)
        f->ip = (uint16_t)k;
      s = CVM_OK;
      break;
    case OP_CALL: {
      unsigned fn = p->code[f->ip++], argc = p->code[f->ip++],
               base = v->sp - argc;
      CvmFrame *nf;
      if (v->depth >= CVM_FRAMES)
        return CVM_FRAME_LIMIT;
      nf = &v->frames[v->depth++];
      frame(nf, (uint8_t)fn, p->functions[fn].entry, (uint16_t)base);
      for (k = 0; k < argc; k++) {
        nf->locals[k] = v->stack[base + k];
        nf->initialized[k] = 1;
      }
      v->sp = (uint16_t)base;
      s = CVM_OK;
      break;
    }
    case OP_RET:
      r = v->stack[--v->sp];
      v->sp = f->base;
      v->depth--;
      if (!v->depth) {
        *out = r;
        return CVM_OK;
      }
      s = push(v, r);
      break;
    case OP_PAIR:
      b = v->stack[--v->sp];
      a = v->stack[--v->sp];
      s = cvm_pair(v, a, b, &r);
      if (s == CVM_OK)
        s = push(v, r);
      break;
    case OP_FIRST:
    case OP_SECOND:
      a = v->stack[--v->sp];
      s = cvm_field(v, a, op == OP_SECOND, &r);
      if (s == CVM_OK)
        s = push(v, r);
      break;
    case OP_GC:
      s = cvm_collect(v);
      break;
    case OP_HOST:
      k = p->code[f->ip++];
      a = v->stack[--v->sp];
      if (a.tag != CVM_INT)
        return CVM_TYPE;
      if (!v->host_budget)
        return CVM_HOST_BUDGET;
      v->host_budget--;
      if (!v->host)
        return CVM_HOST_ERROR;
      r = cvm_int(0);
      if (v->host(v->host_context, (uint8_t)k, a.integer, &r.integer))
        return CVM_HOST_ERROR;
      s = push(v, r);
      break;
    default:
      return CVM_VERIFY;
    }
    if (s != CVM_OK)
      return s;
  }
  return CVM_VERIFY;
}
