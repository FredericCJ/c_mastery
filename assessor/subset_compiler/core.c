#include "core.h"
#ifndef B06_MUTATION
#define B06_MUTATION 0
#endif

static int cc_put(char *b, size_t *n, size_t cap, const char *s)
{
    size_t i = 0;
    while (s[i])
    {
        if (*n + 1 >= cap)
            return 0;
        b[(*n)++] = s[i++];
    }
    b[*n] = 0;
    return 1;
}

static int cc_num(char *b, size_t *n, size_t cap, uint32_t v)
{
    char a[16];
    unsigned k = 0;
    do
    {
        a[k++] = (char)('0' + v % 10);
        v /= 10;
    } while (v);
    while (k)
    {
        char s[2] = {a[--k], 0};
        if (!cc_put(b, n, cap, s))
            return 0;
    }
    return 1;
}

struct cc_parser
{
    const char *s;
    size_t n, pos, used, cap;
    struct cc_ins *ir;
};

static int cc_add_ins(struct cc_parser *p, unsigned op, uint32_t v)
{
    if (p->used >= p->cap)
        return 0;
    p->ir[p->used].op = op;
    p->ir[p->used++].value = v;
    return 1;
}

static int cc_expr(struct cc_parser *p, unsigned depth);

static void cc_space(struct cc_parser *p)
{
    while (p->pos < p->n && (p->s[p->pos] == ' ' || p->s[p->pos] == '\n' ||
                             p->s[p->pos] == '\t'))
        ++p->pos;
}

static int cc_atom(struct cc_parser *p, unsigned depth)
{
    unsigned kind;
    uint32_t v;
    size_t old = p->pos;
    if (depth > 16 || !cc_token(p->s, p->n, &p->pos, &kind, &v))
        return 0;
    if (kind == 1)
        return cc_add_ins(p, 1, v);
    if (kind == 2)
        return cc_add_ins(p, 2, 0);
    if (kind == 3)
    {
        if (!cc_expr(p, depth + 1))
            return 0;
        if (!cc_token(p->s, p->n, &p->pos, &kind, &v) || kind != 4)
            return 0;
        return 1;
    }
    p->pos = old;
    return 0;
}

static int cc_product(struct cc_parser *p, unsigned depth)
{
    if (!cc_atom(p, depth))
        return 0;
    for (;;)
    {
        cc_space(p);
        if (p->pos >= p->n || p->s[p->pos] != '*')
            return 1;
        ++p->pos;
        if (!cc_atom(p, depth) || !cc_add_ins(p, 4, 0))
            return 0;
    }
}

static int cc_expr(struct cc_parser *p, unsigned depth)
{
    if (!cc_product(p, depth))
        return 0;
    for (;;)
    {
        cc_space(p);
        if (p->pos >= p->n || p->s[p->pos] != '+')
            return 1;
        ++p->pos;
        if (!cc_product(p, depth) || !cc_add_ins(p, 3, 0))
            return 0;
    }
}

static int cc_emit_impl(unsigned target, const struct cc_ins *ir, size_t count,
                        char *out, size_t cap)
{
    char tmp[4096];
    size_t n = 0, i;
    unsigned peak;
    if (!cc_validate(ir, count, &peak) || target > 1)
        return 0;
    if (B06_MUTATION == 7 && target == 1)
    {
        if (!cc_put(
                tmp, &n, sizeof tmp,
                ".syntax unified\n.thumb\n.text\n.globl cm_expr\n.thumb_func\ncm_expr:\n"))
            return 0;
    }
    else if (
        !cc_put(
            tmp, &n, sizeof tmp,
            target == 0
                ? ".text\n.globl cm_expr\ncm_expr:\n"
                : ".syntax unified\n.thumb\n.text\n.globl cm_expr\n.thumb_func\ncm_expr:\nmov r3,r0\n"))
        return 0;
    for (i = 0; i < count; ++i)
    {
        unsigned op = ir[i].op;
        if (target == 0 && B06_MUTATION == 6 && op == 4)
            op = 3;
        if (op == 1)
        {
            if (!cc_put(tmp, &n, sizeof tmp,
                        target == 0 ? "movl $" : "movw r0,#") ||
                !cc_num(tmp, &n, sizeof tmp, ir[i].value) ||
                !cc_put(tmp, &n, sizeof tmp,
                        target == 0 ? ",%eax\npushq %rax\n" : "\npush {r0}\n"))
                return 0;
        }
        else if (op == 2)
        {
            if (!cc_put(tmp, &n, sizeof tmp,
                        target == 0 ? "movl %edi,%eax\npushq %rax\n"
                                    : "mov r0,r3\npush {r0}\n"))
                return 0;
        }
        else if (op == 3 || op == 4)
        {
            const char *s =
                target == 0
                    ? (op == 3
                           ? "popq %rax\npopq %rcx\naddl %ecx,%eax\npushq %rax\n"
                           : "popq %rax\npopq %rcx\nimull %ecx,%eax\npushq %rax\n")
                    : (op == 3 ? "pop {r0,r1}\nadds r0,r1,r0\npush {r0}\n"
                               : "pop {r0,r1}\nmul r0,r1,r0\npush {r0}\n");
            if (!cc_put(tmp, &n, sizeof tmp, s))
                return 0;
        }
        else if (!cc_put(tmp, &n, sizeof tmp,
                         target == 0 ? "popq %rax\nret\n"
                                     : "pop {r0}\nbx lr\n"))
            return 0;
    }
    if (n + 1 > cap)
        return 0;
    for (i = 0; i <= n; ++i)
        out[i] = tmp[i];
    return 1;
}

int cc_token(const char *s, size_t n, size_t *off, unsigned *kind,
             uint32_t *value)
{
    size_t i = *off;
    unsigned k = 0;
    uint32_t v = 0;
    if (i > n)
        return 0;
    while (i < n && (s[i] == ' ' || s[i] == '\n' || s[i] == '\t'))
        ++i;
    if (i < n)
    {
        if (s[i] >= '0' && s[i] <= '9')
        {
            k = 1;
            do
            {
                unsigned d = (unsigned)(s[i] - '0');
                if (v > (65535u - d) / 10u && B06_MUTATION != 1)
                    return 0;
                v = v * 10u + d;
                ++i;
            } while (i < n && s[i] >= '0' && s[i] <= '9');
        }
        else
        {
            char c = s[i++];
            k = c == 'x'   ? 2
                : c == '(' ? 3
                : c == ')' ? 4
                : c == '+' ? 5
                : c == '*' ? 6
                           : 99;
            if (k == 99)
                return 0;
        }
    }
    *off = i;
    *kind = k;
    *value = v;
    return 1;
}

int cc_parse(const char *s, size_t n, struct cc_ins *ir, size_t cap,
             size_t *used)
{
    struct cc_parser p = {s, n, 0, 0, cap, ir};
    if (!cc_expr(&p, 0))
        return 0;
    cc_space(&p);
    if ((p.pos != n && B06_MUTATION != 2) || !cc_add_ins(&p, 5, 0))
        return 0;
    *used = p.used;
    return 1;
}

int cc_binary(unsigned op, uint32_t a, uint32_t b, uint32_t *o)
{
    if (op != 3 && op != 4)
        return 0;
    *o = op == 3 ? (uint32_t)((uint64_t)a + b) : (uint32_t)((uint64_t)a * b);
    if (B06_MUTATION == 3 && op == 4)
        *o = a + b;
    return 1;
}

int cc_run(const struct cc_ins *ir, size_t n, uint32_t x, uint32_t *out)
{
    uint32_t s[64];
    size_t i, sp = 0;
    unsigned peak;
    if (!cc_validate(ir, n, &peak))
        return 0;
    for (i = 0; i < n; ++i)
    {
        unsigned op = ir[i].op;
        if (op == 1)
            s[sp++] = ir[i].value;
        else if (op == 2)
            s[sp++] = x;
        else if (op == 3 || op == 4)
        {
            uint32_t b = s[--sp], a = s[--sp], v;
            if (!cc_binary(op, a, b, &v))
                return 0;
            s[sp++] = v;
        }
        else
        {
            *out = B06_MUTATION == 4 ? 0 : s[0];
            return 1;
        }
    }
    return 0;
}

int cc_validate(const struct cc_ins *ir, size_t n, unsigned *peak)
{
    size_t i;
    unsigned sp = 0, max = 0;
    if (n == 0 || n > 128)
        return 0;
    for (i = 0; i < n; ++i)
    {
        unsigned op = ir[i].op;
        if (op == 1 || op == 2)
        {
            if ((op == 1 && ir[i].value > 65535) || sp == 64)
                return 0;
            ++sp;
            if (sp > max)
                max = sp;
        }
        else if (op == 3 || op == 4)
        {
            if (sp < 2)
                return 0;
            --sp;
        }
        else if (op == 5)
        {
            if (sp != 1 || (i + 1 != n && B06_MUTATION != 5))
                return 0;
            *peak = max;
            return 1;
        }
        else
            return 0;
    }
    return 0;
}

int cc_x86(const struct cc_ins *ir, size_t n, char *o, size_t cap)
{
    return cc_emit_impl(0, ir, n, o, cap);
}

int cc_thumb(const struct cc_ins *ir, size_t n, char *o, size_t cap)
{
    return cc_emit_impl(1, ir, n, o, cap);
}

int cc_compile_eval(const char *s, size_t n, uint32_t x, uint32_t *o)
{
    struct cc_ins ir[128];
    size_t used;
    uint32_t v;
    if (!cc_parse(s, n, ir, 128, &used) || !cc_run(ir, used, x, &v))
        return 0;
    if (B06_MUTATION == 8)
        v += 1;
    *o = v;
    return 1;
}
