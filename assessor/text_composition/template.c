#include "text.h"
#ifndef TC_MUTANT
#define TC_MUTANT 0
#endif
static int add(struct tc_program *p, unsigned kind, unsigned value, unsigned jump) {
  if (p->count == 128)
    return 0;
  p->ops[p->count++] = (struct tc_op){kind, value, jump};
  return 1;
}
int tc_template_compile(const unsigned char *s, size_t length, struct tc_program *out) {
  if (!out || (!s && length))
    return TC_INVALID;
  if (length > 256)
    return TC_RANGE;
  struct tc_program p = {0};
  unsigned stack[64], depth = 0;
  for (size_t i = 0; i < length;) {
    if (s[i] != '{' && s[i] != '}') {
      if ((s[i] < 32 && s[i] != '\n' && s[i] != '\t') || s[i] > 126)
        return TC_SYNTAX;
      if (!add(&p, TT_LITERAL, s[i++], 0))
        return TC_RANGE;
      continue;
    }
    if (i + 1 < length && s[i] == s[i + 1]) {
      if (!add(&p, TT_LITERAL, s[i], 0))
        return TC_RANGE;
      i += 2;
      continue;
    }
    if (s[i] != '{')
      return TC_SYNTAX;
    if (i + 2 < length && s[i + 2] == '}') {
      unsigned char c = s[i + 1];
      if (c == '/') {
        if (!depth)
          return TC_SYNTAX;
        unsigned start = stack[--depth];
        if (!add(&p, TT_END, start, 0))
          return TC_RANGE;
        p.ops[start].jump = p.count;
      } else if ((c >= 'a' && c <= 'd') || c == '.') {
        if (!add(&p, TT_VALUE, c == '.' ? 4 : (unsigned)(c - 'a'), 0))
          return TC_RANGE;
      } else
        return TC_SYNTAX;
      i += 3;
      continue;
    }
    if (i + 3 < length && s[i + 3] == '}') {
      unsigned char op = s[i + 1], key = s[i + 2];
      if (op == '>' && key >= '0' && key <= '3') {
        if (!add(&p, TT_INCLUDE, (unsigned)(key - '0'), 0))
          return TC_RANGE;
      } else if ((op == '?' || op == '*') && key >= 'a' && key <= 'd') {
        if (depth == 64)
          return TC_RANGE;
        stack[depth++] = p.count;
        if (!add(&p, op == '?' ? TT_IF : TT_LOOP, (unsigned)(key - 'a'), 0))
          return TC_RANGE;
      } else
        return TC_SYNTAX;
      i += 4;
      continue;
    }
    return TC_SYNTAX;
  }
  if (depth)
    return TC_SYNTAX;
  *out = p;
  return TC_OK;
}
struct tc_run {
  const struct tc_program *bank;
  unsigned count, html, budget;
  const struct tc_model *model;
  struct tc_text result;
};
static int tick(struct tc_run *r) {
  if (!r->budget)
    return 0;
  --r->budget;
  return 1;
}
static int byte(struct tc_run *r, unsigned char ch) {
  if (!tick(r))
    return TC_FUEL;
  if (r->result.length == 4096)
    return TC_RANGE;
  r->result.bytes[r->result.length++] = ch;
  return TC_OK;
}
static int value(struct tc_run *r, const struct tc_value *v) {
  for (size_t i = 0; i < v->length; i++) {
    const char *replacement = 0;
    unsigned char c = v->bytes[i];
    if (r->html && TC_MUTANT != 5) {
      if (c == '&')
        replacement = "&amp;";
      else if (c == '<')
        replacement = "&lt;";
      else if (c == '>')
        replacement = "&gt;";
      else if (c == '\"')
        replacement = "&quot;";
      else if (c == '\'')
        replacement = "&#39;";
    }
    if (replacement) {
      for (size_t j = 0; replacement[j]; j++) {
        int status = byte(r, (unsigned char)replacement[j]);
        if (status)
          return status;
      }
    } else {
      int status = byte(r, c);
      if (status)
        return status;
    }
  }
  return TC_OK;
}
static int execute(struct tc_run *r, unsigned program, unsigned start, unsigned end,
                   const struct tc_value *current, unsigned active, unsigned depth) {
  if (depth > 128)
    return TC_RANGE;
  const struct tc_program *p = r->bank + program;
  for (unsigned i = start; i < end;) {
    if (!tick(r))
      return TC_FUEL;
    const struct tc_op *op = p->ops + i;
    int status = TC_OK;
    if (op->kind == TT_LITERAL) {
      status = byte(r, (unsigned char)op->value);
      ++i;
    } else if (op->kind == TT_VALUE) {
      if (op->value == 4) {
        if (current)
          status = value(r, current);
      } else if (r->model->fields[op->value].count)
        status = value(r, &r->model->fields[op->value].values[0]);
      ++i;
    } else if (op->kind == TT_INCLUDE) {
      if (op->value >= r->count)
        return TC_INVALID;
      if (active & (1u << op->value))
        return TC_CYCLE;
      status = execute(r, op->value, 0, r->bank[op->value].count, current,
                       active | (1u << op->value), depth + 1);
      ++i;
    } else if (op->kind == TT_IF || op->kind == TT_LOOP) {
      const struct tc_field *f = r->model->fields + op->value;
      if (op->kind == TT_IF) {
        if (f->count)
          status = execute(r, program, i + 1, op->jump - 1, current, active,
                           depth + 1);
      } else
        for (unsigned j = 0; j < f->count; j++) {
          if (!tick(r))
            return TC_FUEL;
          status =
              execute(r, program, i + 1, op->jump - 1,
                      &f->values[TC_MUTANT == 6 ? 0 : j], active, depth + 1);
          if (status)
            break;
        }
      i = op->jump;
    } else
      return TC_INVALID;
    if (status)
      return status;
  }
  return TC_OK;
}
static int valid(const struct tc_program *p) {
  if (p->count > 128)
    return 0;
  unsigned stack[64], depth = 0;
  for (unsigned i = 0; i < p->count; i++) {
    const struct tc_op *o = p->ops + i;
    if (o->kind > TT_INCLUDE)
      return 0;
    if (o->kind == TT_LITERAL &&
        (o->value > 126 ||
         (o->value < 32 && o->value != '\n' && o->value != '\t')))
      return 0;
    if (o->kind == TT_VALUE && o->value > 4)
      return 0;
    if (o->kind == TT_INCLUDE && o->value > 3)
      return 0;
    if (o->kind == TT_IF || o->kind == TT_LOOP) {
      if (o->value > 3 || depth == 64 || o->jump <= i + 1 || o->jump > p->count)
        return 0;
      stack[depth++] = i;
    }
    if (o->kind == TT_END) {
      if (!depth)
        return 0;
      unsigned start = stack[--depth];
      if (o->value != start || p->ops[start].jump != i + 1)
        return 0;
    }
  }
  return !depth;
}
int tc_template_render(const struct tc_program *bank, unsigned count, unsigned entry,
                       const struct tc_model *model, unsigned html_text,
                       unsigned budget, struct tc_text *out) {
  if (!bank || !model || !out || count == 0 || count > 4 || entry >= count ||
      html_text > 1)
    return TC_INVALID;
  for (unsigned i = 0; i < count; i++)
    if (!valid(bank + i))
      return TC_INVALID;
  for (unsigned i = 0; i < 4; i++) {
    const struct tc_field *f = model->fields + i;
    if (f->count > 4)
      return TC_INVALID;
    for (unsigned j = 0; j < f->count; j++) {
      const struct tc_value *v = f->values + j;
      if (v->length > 256)
        return TC_INVALID;
      for (size_t k = 0; k < v->length; k++)
        if ((v->bytes[k] < 32 && v->bytes[k] != '\n' && v->bytes[k] != '\t') ||
            v->bytes[k] > 126)
          return TC_INVALID;
    }
  }
  struct tc_run r = {bank, count, html_text, budget, model, {{0}, 0}};
  int status = execute(&r, entry, 0, bank[entry].count, 0, 1u << entry, 0);
  if (!status)
    *out = r.result;
  return status;
}
