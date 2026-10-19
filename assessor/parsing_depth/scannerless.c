#include "parsing.h"
#ifndef PD_MUTANT
#define PD_MUTANT 0
#endif
typedef struct {
  int status;
  size_t end;
} Match;
typedef struct {
  const PsNode *nodes;
  const unsigned char *input;
  size_t length;
  unsigned eof, budget;
  PsResult result;
  PdDiagnostic *d;
} Parse;
static Match match(Parse *p, unsigned index, size_t pos) {
  if (!p->budget)
    return (Match){PD_FUEL, pos};
  --p->budget;
  ++p->d->steps;
  const PsNode *n = &p->nodes[index];
  unsigned saved = p->result.count;
  if (n->kind <= PS_ANY) {
    if (pos == p->length) {
      if (pos >= p->d->offset) {
        p->d->offset = pos;
        p->d->expected = n->low;
      }
      return (Match){p->eof || PD_MUTANT == 7 ? PD_NO_MATCH : PD_PENDING, pos};
    }
    unsigned c = p->input[pos];
    if (n->kind == PS_ANY || (n->kind == PS_BYTE && c == n->low) ||
        (n->kind == PS_RANGE && c >= n->low && c <= n->high))
      return (Match){PD_OK, pos + 1};
    if (pos >= p->d->offset) {
      p->d->offset = pos;
      p->d->expected = n->low;
    }
    return (Match){PD_NO_MATCH, pos};
  }
  if (n->kind == PS_EMPTY)
    return (Match){PD_OK, pos};
  if (n->kind == PS_STAR) {
    size_t end = pos;
    for (;;) {
      unsigned before = p->result.count;
      Match r = match(p, n->left, end);
      if (r.status == PD_NO_MATCH) {
        p->result.count = before;
        return (Match){PD_OK, end};
      }
      if (r.status) {
        p->result.count = saved;
        return r;
      }
      if (r.end == end) {
        p->result.count = saved;
        return (Match){PD_EMPTY_LOOP, end};
      }
      end = r.end;
    }
  }
  Match a = match(p, n->left, pos);
  if (n->kind == PS_NOT) {
    p->result.count = saved;
    if (a.status == PD_OK)
      return (Match){PD_NO_MATCH, pos};
    if (a.status == PD_NO_MATCH)
      return (Match){PD_OK, pos};
    return a;
  }
  if (n->kind == PS_CAPTURE) {
    if (a.status) {
      p->result.count = saved;
      return a;
    }
    if (p->result.count == 32) {
      p->result.count = saved;
      return (Match){PD_RANGE, pos};
    }
    p->result.captures[p->result.count++] = (PsCapture){n->low, pos, a.end};
    return a;
  }
  if (n->kind == PS_CHOICE) {
    if (a.status != PD_NO_MATCH)
      return a;
    if (PD_MUTANT != 8)
      p->result.count = saved;
    return match(p, n->right, pos);
  }
  if (a.status) {
    if (PD_MUTANT != 8)
      p->result.count = saved;
    return a;
  }
  Match b = match(p, n->right, a.end);
  if (b.status && PD_MUTANT != 8)
    p->result.count = saved;
  return b;
}
int ps_parse(const PsNode *n, size_t count, const unsigned char *input,
             size_t length, unsigned eof, unsigned budget, PsResult *out,
             PdDiagnostic *d) {
  if (!n || (!input && length) || !out || !d || eof > 1)
    return PD_INVALID;
  *d = (PdDiagnostic){0};
  if (!count || count > 64 || length > 256)
    return PD_RANGE;
  for (size_t i = 0; i < count; ++i) {
    unsigned k = n[i].kind;
    if (k > PS_CAPTURE)
      return PD_INVALID;
    if (k == PS_BYTE && n[i].low > 255)
      return PD_INVALID;
    if (k == PS_RANGE && (n[i].low > n[i].high || n[i].high > 255))
      return PD_INVALID;
    if (k >= PS_SEQ && (n[i].left <= i || n[i].left >= count))
      return PD_INVALID;
    if ((k == PS_SEQ || k == PS_CHOICE) &&
        (n[i].right <= i || n[i].right >= count))
      return PD_INVALID;
  }
  Parse p = {.nodes = n,
             .input = input,
             .length = length,
             .eof = eof,
             .budget = budget,
             .d = d};
  Match r = match(&p, 0, 0);
  if (!r.status) {
    p.result.end = r.end;
    *out = p.result;
  }
  return r.status;
}
