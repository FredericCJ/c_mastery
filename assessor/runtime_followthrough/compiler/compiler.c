#include "compiler.h"
#include <limits.h>
#include <string.h>
#ifndef VC_MUTANT
#define VC_MUTANT 0
#endif
enum { LIST = 1, WORD, NUMBER };
static int ws(unsigned char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
static int named(const VcNode *n, const char *s) {
  return n->kind == WORD && !strcmp(n->word, s);
}
static int ident(const VcNode *n) {
  if (n->kind != WORD)
    return 0;
  const char *s = n->word;
  if (!((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || *s == '_'))
    return 0;
  for (unsigned i = 1; s[i]; i++)
    if (!((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') ||
          (s[i] >= '0' && s[i] <= '9') || s[i] == '_'))
      return 0;
  return strcmp(s, "true") && strcmp(s, "false") && strcmp(s, "nil");
}
static unsigned add(VcWork *w, unsigned kind) {
  if (w->count == VC_NODES) {
    w->failed = 1;
    return 0;
  }
  unsigned id = w->count++, level = w->parse_depth;
  w->nodes[id].kind = kind;
  if (w->tails[level])
    w->nodes[w->tails[level]].next = id;
  else
    w->nodes[w->parents[level]].first = id;
  w->tails[level] = id;
  return id;
}
static void parse(VcWork *w, const char *s, size_t n, VcReport *r) {
  w->count = 1;
  w->nodes[0].kind = LIST;
  for (size_t pos = 0; pos < n && !w->failed;) {
    unsigned char c = (unsigned char)s[pos];
    if (ws(c)) {
      pos++;
      continue;
    }
    if (c == ')') {
      if (!w->parse_depth) {
        w->failed = 1;
        break;
      }
      w->parse_depth--;
      pos++;
      continue;
    }
    if (c == '(') {
      unsigned id = add(w, LIST);
      if (w->parse_depth + 1 >= VC_PARSE) {
        w->failed = 1;
        break;
      }
      ++w->parse_depth;
      w->parents[w->parse_depth] = id;
      w->tails[w->parse_depth] = 0;
      if (w->parse_depth > r->parser_peak)
        r->parser_peak = w->parse_depth;
      pos++;
      continue;
    }
    size_t start = pos;
    while (pos < n && !ws((unsigned char)s[pos]) && s[pos] != '(' &&
           s[pos] != ')')
      pos++;
    size_t len = pos - start;
    if (!len || len > 31) {
      w->failed = 1;
      break;
    }
    unsigned id = add(w, WORD);
    if (w->failed)
      break;
    VcNode *v = &w->nodes[id];
    for (size_t i = 0; i < len; i++) {
      c = (unsigned char)s[start + i];
      if (c < 33 || c > 126)
        w->failed = 1;
      v->word[i] = (char)c;
    }
    unsigned negative = v->word[0] == '-' && len > 1, number = 1;
    for (size_t i = negative; i < len; i++)
      if (v->word[i] < '0' || v->word[i] > '9')
        number = 0;
    if (number) {
      uint32_t value = 0, limit = negative ? UINT32_C(2147483648) : INT32_MAX;
      for (size_t i = negative; i < len; i++) {
        unsigned digit = (unsigned)(v->word[i] - '0');
        if (value > (limit - digit) / 10u) {
          w->failed = 1;
          break;
        }
        value = value * 10u + digit;
      }
      if (!w->failed) {
        v->kind = NUMBER;
        v->integer = negative ? (int32_t)(-(int64_t)value) : (int32_t)value;
      }
    }
  }
  if (w->parse_depth)
    w->failed = 1;
  r->nodes = w->count;
}
static unsigned next(VcWork *w, unsigned id) { return w->nodes[id].next; }
static unsigned count(VcWork *w, unsigned id) {
  unsigned n = 0;
  for (; id; id = next(w, id))
    n++;
  return n;
}
static void signatures(VcWork *w) {
  for (unsigned id = w->nodes[0].first; id && !w->failed; id = next(w, id)) {
    unsigned head = w->nodes[id].first, name = next(w, head),
             params = next(w, name), body = next(w, params);
    if (w->nodes[id].kind != LIST || !named(&w->nodes[head], "fn") ||
        !ident(&w->nodes[name]) || w->nodes[params].kind != LIST || !body ||
        next(w, body) || w->staged.count == CVM_FUNCTIONS) {
      w->failed = 1;
      break;
    }
    for (unsigned i = 0; i < w->staged.count; i++)
      if (!strcmp(w->nodes[w->definitions[i].name].word, w->nodes[name].word))
        w->failed = 1;
    unsigned argc = count(w, w->nodes[params].first);
    if (argc > CVM_LOCALS)
      w->failed = 1;
    for (unsigned a = w->nodes[params].first; a; a = next(w, a)) {
      if (!ident(&w->nodes[a]))
        w->failed = 1;
      for (unsigned b = w->nodes[params].first; b != a; b = next(w, b))
        if (!strcmp(w->nodes[a].word, w->nodes[b].word))
          w->failed = 1;
    }
    if (!w->failed)
      w->definitions[w->staged.count++] =
          (VcDefinition){name, params, body, argc};
  }
  if (!w->staged.count || !named(&w->nodes[w->definitions[0].name], "main") ||
      w->definitions[0].arity)
    w->failed = 1;
  for (unsigned id = 1; id < w->count; id++)
    if (w->nodes[id].kind == LIST &&
        named(&w->nodes[w->nodes[id].first], "break"))
      w->breaks = 1;
}
static void byte(VcWork *w, unsigned n) {
  if (w->staged.length >= CVM_CODE)
    w->failed = 1;
  else
    w->staged.code[w->staged.length++] = (uint8_t)n;
}
static void constant(VcWork *w, int32_t n) {
  uint32_t u = (uint32_t)n;
  byte(w, OP_INT);
  for (unsigned i = 0; i < 4; i++) {
    byte(w, u & 255u);
    u >>= 8;
  }
  w->height++;
}
static unsigned branch(VcWork *w, unsigned op) {
  byte(w, op);
  unsigned p = w->staged.length;
  byte(w, 0);
  byte(w, 0);
  return p;
}
static void patch(VcWork *w, unsigned at, unsigned target) {
  if (at + 1 >= CVM_CODE || target > CVM_CODE) {
    w->failed = 1;
    return;
  }
  if (VC_MUTANT == 3)
    target++;
  w->staged.code[at] = (uint8_t)target;
  w->staged.code[at + 1] = (uint8_t)(target >> 8);
}
static int local(VcWork *w, unsigned id) {
  if (VC_MUTANT == 1) {
    for (unsigned i = 0; i < w->used; i++)
      if (!strcmp(w->names[i], w->nodes[id].word))
        return (int)i;
    return -1;
  }
  unsigned i = w->used;
  while (i)
    if (!strcmp(w->names[--i], w->nodes[id].word))
      return (int)i;
  return -1;
}
static void clear(VcWork *w, unsigned slot) {
  byte(w, OP_NIL);
  byte(w, OP_STORE);
  byte(w, slot);
}
static void push(VcWork *w, unsigned node, int loop, VcReport *r) {
  if (!node || w->sp >= w->limit) {
    w->failed = 1;
    return;
  }
  VcFrame f = {0};
  f.node = node;
  f.base = w->height;
  f.used = w->used;
  f.loop = loop;
  f.head = UINT_MAX;
  w->frames[w->sp++] = f;
  if (w->sp > r->emitter_peak)
    r->emitter_peak = w->sp;
}
static void done(VcWork *w) {
  w->height = w->frames[w->sp - 1].base + 1;
  w->sp--;
}
static unsigned binary(const char *s) {
  return !strcmp(s, "+")      ? OP_ADD
         : !strcmp(s, "-")    ? OP_SUB
         : !strcmp(s, "*")    ? OP_MUL
         : !strcmp(s, "/")    ? OP_DIV
         : !strcmp(s, "==")   ? OP_EQ
         : !strcmp(s, "<")    ? OP_LT
         : !strcmp(s, "pair") ? OP_PAIR
                              : 0;
}
static void compile_body(VcWork *w, unsigned body, const VcOptions *o,
                         VcReport *r) {
  push(w, body, -1, r);
  while (w->sp && !w->failed) {
    VcFrame *f = &w->frames[w->sp - 1];
    VcNode *v = &w->nodes[f->node];
    unsigned h = v->first, a = next(w, h), b = next(w, a), c = next(w, b);
    const char *op = w->nodes[h].word;
    if (f->stage == 0) {
      if (v->kind == NUMBER) {
        constant(w, v->integer);
        done(w);
        continue;
      }
      if (v->kind == WORD) {
        if (named(v, "true") || named(v, "false") || named(v, "nil"))
          byte(w, named(v, "true")    ? OP_TRUE
                  : named(v, "false") ? OP_FALSE
                                      : OP_NIL);
        else {
          int at = local(w, f->node);
          if (at < 0) {
            w->failed = 1;
            continue;
          }
          byte(w, OP_LOAD);
          byte(w, (unsigned)at);
        }
        w->height++;
        done(w);
        continue;
      }
      if (v->kind != LIST || w->nodes[h].kind != WORD) {
        w->failed = 1;
        continue;
      }
      unsigned n = count(w, a), code = binary(op);
      if (code) {
        if (n != 2) {
          w->failed = 1;
          continue;
        }
        if ((o->fold_constants || VC_MUTANT == 6) &&
            w->nodes[a].kind == NUMBER && w->nodes[b].kind == NUMBER &&
            (code == OP_ADD || code == OP_SUB || code == OP_MUL)) {
          int64_t x = w->nodes[a].integer, y = w->nodes[b].integer,
                  z = code == OP_ADD   ? x + y
                      : code == OP_SUB ? x - y
                                       : x * y;
          if (z >= INT32_MIN && z <= INT32_MAX) {
            constant(w, (int32_t)z);
            w->folds++;
            done(w);
            continue;
          }
        }
        f->slot = code;
        f->stage = 100;
        push(w, a, f->loop, r);
        continue;
      }
      if (!strcmp(op, "let")) {
        if (n != 3 || !ident(&w->nodes[a]) ||
            w->used >= CVM_LOCALS - w->breaks) {
          w->failed = 1;
          continue;
        }
        f->slot = w->used;
        f->stage = 10;
        if (VC_MUTANT == 2) {
          strcpy(w->names[w->used++], w->nodes[a].word);
          if (w->used > w->peak)
            w->peak = w->used;
        }
        push(w, b, f->loop, r);
        continue;
      }
      if (!strcmp(op, "set")) {
        int at = local(w, a);
        if (n != 2 || !ident(&w->nodes[a]) || at < 0) {
          w->failed = 1;
          continue;
        }
        f->slot = (unsigned)at;
        f->stage = 20;
        push(w, b, f->loop, r);
        continue;
      }
      if (!strcmp(op, "if")) {
        if (n != 3) {
          w->failed = 1;
          continue;
        }
        f->stage = 30;
        push(w, a, f->loop, r);
        continue;
      }
      if (!strcmp(op, "while")) {
        if (n != 2) {
          w->failed = 1;
          continue;
        }
        f->a = w->staged.length;
        f->stage = 40;
        push(w, a, f->loop, r);
        continue;
      }
      if (!strcmp(op, "do") || !strcmp(op, "block")) {
        if (!n || (!strcmp(op, "do") && n != 2)) {
          w->failed = 1;
          continue;
        }
        f->cursor = a;
        f->stage = 50;
        push(w, a, f->loop, r);
        continue;
      }
      if (!strcmp(op, "call")) {
        if (!a || w->nodes[a].kind != WORD) {
          w->failed = 1;
          continue;
        }
        unsigned index = w->staged.count;
        for (unsigned j = 0; j < w->staged.count; j++)
          if (!strcmp(w->nodes[w->definitions[j].name].word, w->nodes[a].word))
            index = j;
        if (index == w->staged.count || n - 1 != w->definitions[index].arity) {
          w->failed = 1;
          continue;
        }
        f->slot = index;
        f->cursor = b;
        f->stage = 60;
        if (b)
          push(w, b, f->loop, r);
        continue;
      }
      if (!strcmp(op, "host")) {
        if (n != 2 || w->nodes[a].kind != NUMBER || w->nodes[a].integer < 0 ||
            w->nodes[a].integer > 255) {
          w->failed = 1;
          continue;
        }
        f->slot = (unsigned)w->nodes[a].integer;
        f->stage = 70;
        push(w, b, f->loop, r);
        continue;
      }
      if (!strcmp(op, "first") || !strcmp(op, "second")) {
        if (n != 1) {
          w->failed = 1;
          continue;
        }
        f->slot = !strcmp(op, "first") ? OP_FIRST : OP_SECOND;
        f->stage = 80;
        push(w, a, f->loop, r);
        continue;
      }
      if (!strcmp(op, "gc")) {
        if (n) {
          w->failed = 1;
          continue;
        }
        byte(w, OP_GC);
        byte(w, OP_NIL);
        w->height++;
        done(w);
        continue;
      }
      if (!strcmp(op, "and") || !strcmp(op, "or")) {
        if (n != 2) {
          w->failed = 1;
          continue;
        }
        f->slot = !strcmp(op, "or");
        f->stage = 90;
        push(w, a, f->loop, r);
        continue;
      }
      if (!strcmp(op, "break")) {
        if (n != 1 || f->loop < 0) {
          w->failed = 1;
          continue;
        }
        f->stage = 110;
        push(w, a, f->loop, r);
        continue;
      }
      w->failed = 1;
      continue;
    }
    switch (f->stage) {
    case 10:
      if (VC_MUTANT != 2) {
        strcpy(w->names[w->used++], w->nodes[a].word);
        if (w->used > w->peak)
          w->peak = w->used;
      }
      byte(w, OP_STORE);
      byte(w, f->slot);
      w->height--;
      f->stage = 11;
      push(w, c, f->loop, r);
      break;
    case 11:
      clear(w, f->slot);
      w->used = f->used;
      done(w);
      break;
    case 20:
      byte(w, OP_STORE);
      byte(w, f->slot);
      byte(w, OP_LOAD);
      byte(w, f->slot);
      done(w);
      break;
    case 30:
      f->a = branch(w, OP_JFALSE);
      w->height--;
      f->stage = 31;
      push(w, b, f->loop, r);
      break;
    case 31:
      f->b = branch(w, OP_JMP);
      patch(w, f->a, w->staged.length);
      w->height = f->base;
      f->stage = 32;
      push(w, c, f->loop, r);
      break;
    case 32:
      patch(w, f->b, w->staged.length);
      done(w);
      break;
    case 40:
      f->b = branch(w, OP_JFALSE);
      w->height--;
      f->stage = 41;
      push(w, b, (int)w->sp - 1, r);
      break;
    case 41: {
      byte(w, OP_POP);
      w->height--;
      unsigned at = branch(w, OP_JMP);
      patch(w, at, f->a);
      patch(w, f->b, w->staged.length);
      byte(w, OP_NIL);
      for (at = f->head; at != UINT_MAX; at = w->next_patch[at])
        patch(w, at, w->staged.length);
      done(w);
      break;
    }
    case 50:
      f->cursor = next(w, f->cursor);
      if (f->cursor) {
        byte(w, OP_POP);
        w->height--;
        push(w, f->cursor, f->loop, r);
      } else
        done(w);
      break;
    case 60:
      if (f->cursor)
        f->cursor = next(w, f->cursor);
      if (f->cursor)
        push(w, f->cursor, f->loop, r);
      else {
        byte(w, OP_CALL);
        byte(w, f->slot);
        byte(w, w->definitions[f->slot].arity);
        done(w);
      }
      break;
    case 70:
      byte(w, OP_HOST);
      byte(w, f->slot);
      done(w);
      break;
    case 80:
      byte(w, f->slot);
      done(w);
      break;
    case 90:
      if (VC_MUTANT == 4) {
        byte(w, OP_POP);
        w->height--;
        f->stage = 93;
        push(w, b, f->loop, r);
        break;
      }
      f->a = branch(w, OP_JFALSE);
      w->height--;
      if (f->slot) {
        byte(w, OP_TRUE);
        f->b = branch(w, OP_JMP);
        patch(w, f->a, w->staged.length);
        w->height = f->base;
      }
      f->stage = 91;
      push(w, b, f->loop, r);
      break;
    case 91:
      /* Re-test RHS Boolean; integers are not guest truth values. */
      f->cursor = branch(w, OP_JFALSE);
      byte(w, OP_TRUE);
      f->used = branch(w, OP_JMP);
      patch(w, f->cursor, w->staged.length);
      byte(w, OP_FALSE);
      patch(w, f->used, w->staged.length);
      if (f->slot)
        patch(w, f->b, w->staged.length);
      else {
        f->b = branch(w, OP_JMP);
        patch(w, f->a, w->staged.length);
        byte(w, OP_FALSE);
        patch(w, f->b, w->staged.length);
      }
      done(w);
      break;
    case 93:
      done(w);
      break;
    case 100:
      f->stage = 101;
      push(w, b, f->loop, r);
      break;
    case 101:
      byte(w, f->slot);
      done(w);
      break;
    case 110: {
      VcFrame *loop = &w->frames[f->loop];
      byte(w, OP_STORE);
      byte(w, 7);
      w->height--;
      while (w->height > loop->base) {
        byte(w, OP_POP);
        w->height--;
      }
      if (w->height != loop->base) {
        w->failed = 1;
        break;
      }
      for (unsigned i = loop->used; i < w->used; i++)
        clear(w, i);
      if (VC_MUTANT == 5)
        byte(w, OP_NIL);
      else {
        byte(w, OP_LOAD);
        byte(w, 7);
      }
      clear(w, 7);
      unsigned at = branch(w, OP_JMP);
      if (at >= CVM_CODE) {
        w->failed = 1;
        break;
      }
      w->next_patch[at] = loop->head;
      loop->head = at;
      done(w);
      break;
    }
    default:
      w->failed = 1;
      break;
    }
  }
}
CvmStatus vc_compile(const char *s, size_t n, const VcOptions *o, VcWork *w,
                     CvmProgram *out, VcReport *r) {
  if (!s || !o || !w || !out || !r || n > 4096 || !o->expression_depth ||
      o->expression_depth > VC_DEPTH || o->fold_constants > 1)
    return CVM_SYNTAX;
  memset(w, 0, sizeof *w);
  memset(r, 0, sizeof *r);
  w->limit = o->expression_depth;
  parse(w, s, n, r);
  if (!w->failed)
    signatures(w);
  for (unsigned f = 0; f < w->staged.count && !w->failed; f++) {
    VcDefinition *d = &w->definitions[f];
    CvmFunction *fn = &w->staged.functions[f];
    fn->entry = w->staged.length;
    fn->arity = (uint8_t)d->arity;
    w->used = d->arity;
    w->peak = w->used;
    w->height = 0;
    w->sp = 0;
    if (w->used > CVM_LOCALS - w->breaks) {
      w->failed = 1;
      break;
    }
    unsigned i = 0;
    for (unsigned p = w->nodes[d->params].first; p; p = next(w, p))
      strcpy(w->names[i++], w->nodes[p].word);
    compile_body(w, d->body, o, r);
    byte(w, OP_RET);
    fn->length = (uint16_t)(w->staged.length - fn->entry);
    fn->locals = (uint8_t)(w->breaks ? 8 : w->peak);
    if (fn->locals > r->local_peak)
      r->local_peak = fn->locals;
  }
  r->code_bytes = w->staged.length;
  r->folded = w->folds;
  if (w->failed ||
      vv_verify(&w->staged, 0, 1, &w->verification, &r->verifier) != VV_OK ||
      cvm_verify(&w->staged) != CVM_OK) {
    if (VC_MUTANT == 7)
      *out = w->staged;
    return CVM_SYNTAX;
  }
  *out = w->staged;
  return CVM_OK;
}
