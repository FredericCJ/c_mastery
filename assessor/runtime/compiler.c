#include "cvm.h"
#include <limits.h>
#include <string.h>

/* Prefix syntax keeps this teaching compiler small. It is not a C/Lox parser.
 */
enum
{
    END,
    LEFT,
    RIGHT,
    WORD,
    NUMBER,
    BAD
};

struct cvm_token
{
    int kind;
    char word[32];
    int32_t number;
    size_t begin, end;
};

struct cvm_scan
{
    const char *s;
    size_t n, pos;
    struct cvm_token t;
};

struct cvm_definition
{
    char name[32], params[CVM_LOCALS][32];
    uint8_t arity;
    size_t start, end;
};

struct cvm_compiler
{
    struct cvm_program p;
    struct cvm_definition defs[CVM_FUNCTIONS];
    struct cvm_scan scan;
    char names[CVM_LOCALS][32];
    unsigned used, peak, depth;
    int failed;
};

static int space(char c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

static void next(struct cvm_scan *s)
{
    size_t b, k;
    uint64_t n = 0, limit;
    int neg = 0, number = 1;
    s->t.kind = END;
    while (s->pos < s->n && space(s->s[s->pos]))
    {
        s->pos++;
    }
    s->t.begin = s->pos;
    if (s->pos == s->n)
    {
        s->t.end = s->pos;
        return;
    }
    if (s->s[s->pos] == '(' || s->s[s->pos] == ')')
    {
        s->t.kind = s->s[s->pos++] == '(' ? LEFT : RIGHT;
        s->t.end = s->pos;
        return;
    }
    b = s->pos;
    while (s->pos < s->n && !space(s->s[s->pos]) && s->s[s->pos] != '(' &&
           s->s[s->pos] != ')')
    {
        s->pos++;
    }
    k = s->pos - b;
    s->t.end = s->pos;
    if (!k || k >= sizeof s->t.word)
    {
        s->t.kind = BAD;
        return;
    }
    /* Source is a counted byte span. Never let an embedded NUL truncate a
       later C-string comparison, and reject bytes outside the ASCII grammar. */
    for (size_t i = 0; i < k; i++)
    {
        unsigned char byte = (unsigned char)s->s[b + i];
        if (byte < 33 || byte > 126)
        {
            s->t.kind = BAD;
            return;
        }
    }
    memcpy(s->t.word, s->s + b, k);
    s->t.word[k] = 0;
    if (s->t.word[0] == '-' && k > 1)
    {
        neg = 1;
    }
    for (size_t i = (size_t)neg; i < k; i++)
    {
        if (s->t.word[i] < '0' || s->t.word[i] > '9')
        {
            number = 0;
        }
    }
    if (!number)
    {
        s->t.kind = WORD;
        return;
    }
    limit = neg ? UINT64_C(2147483648) : INT32_MAX;
    for (size_t i = (size_t)neg; i < k; i++)
    {
        unsigned d = (unsigned)(s->t.word[i] - '0');
        if (n > (limit - d) / 10u)
        {
            s->t.kind = BAD;
            return;
        }
        n = n * 10u + d;
    }
    s->t.number = neg ? (int32_t)(-(int64_t)n) : (int32_t)n;
    s->t.kind = NUMBER;
}

static int named(const struct cvm_token *t, const char *s)
{
    return t->kind == WORD && !strcmp(t->word, s);
}

static int ident(const char *s)
{
    size_t i;
    if (!((*s >= 'a' && *s <= 'z') || (*s >= 'A' && *s <= 'Z') || *s == '_'))
    {
        return 0;
    }
    for (i = 1; s[i]; i++)
    {
        if (!((s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z') ||
              (s[i] >= '0' && s[i] <= '9') || s[i] == '_'))
        {
            return 0;
        }
    }
    return strcmp(s, "true") && strcmp(s, "false") && strcmp(s, "nil");
}

static int take(struct cvm_compiler *c, int kind)
{
    if (c->scan.t.kind != kind)
    {
        c->failed = 1;
        return 0;
    }
    next(&c->scan);
    return 1;
}

static void emit(struct cvm_compiler *c, unsigned n)
{
    if (c->p.length >= CVM_CODE)
    {
        c->failed = 1;
        return;
    }
    c->p.code[c->p.length++] = (uint8_t)n;
}

static void constant(struct cvm_compiler *c, int32_t n)
{
    uint32_t u = (uint32_t)n;
    emit(c, OP_INT);
    for (unsigned i = 0; i < 4; i++)
    {
        emit(c, u & 255u);
        u >>= 8;
    }
}

static unsigned branch(struct cvm_compiler *c, unsigned op)
{
    unsigned at;
    emit(c, op);
    at = c->p.length;
    emit(c, 0);
    emit(c, 0);
    return at;
}

static void patch(struct cvm_compiler *c, unsigned at, unsigned dest)
{
    if (at + 1 >= CVM_CODE || dest > CVM_CODE)
    {
        c->failed = 1;
        return;
    }
    c->p.code[at] = (uint8_t)dest;
    c->p.code[at + 1] = (uint8_t)(dest >> 8);
}

static int local(struct cvm_compiler *c, const char *name)
{
    unsigned i = c->used;
    while (i)
    {
        if (!strcmp(c->names[--i], name))
        {
            return (int)i;
        }
    }
    return -1;
}

static void expression(struct cvm_compiler *c)
{
    char op[32];
    unsigned a, b, slot, argc;
    int index;
    if (c->failed)
    {
        return;
    }
    if (++c->depth > 64)
    {
        c->failed = 1;
        c->depth--;
        return;
    }
    if (c->scan.t.kind == NUMBER)
    {
        constant(c, c->scan.t.number);
        next(&c->scan);
        goto done;
    }
    if (c->scan.t.kind == WORD)
    {
        if (named(&c->scan.t, "true"))
        {
            emit(c, OP_TRUE);
        }
        else if (named(&c->scan.t, "false"))
        {
            emit(c, OP_FALSE);
        }
        else if (named(&c->scan.t, "nil"))
        {
            emit(c, OP_NIL);
        }
        else
        {
            index = local(c, c->scan.t.word);
            if (index < 0)
            {
                c->failed = 1;
            }
            else
            {
                emit(c, OP_LOAD);
                emit(c, (unsigned)index);
            }
        }
        next(&c->scan);
        goto done;
    }
    if (!take(c, LEFT) || c->scan.t.kind != WORD)
    {
        c->failed = 1;
        goto done;
    }
    strcpy(op, c->scan.t.word);
    next(&c->scan);
    if (!strcmp(op, "let"))
    {
        char name[32];
        if (c->scan.t.kind != WORD || !ident(c->scan.t.word) ||
            c->used >= CVM_LOCALS)
        {
            c->failed = 1;
            goto done;
        }
        strcpy(name, c->scan.t.word);
        next(&c->scan);
        expression(c);
        slot = c->used++;
        strcpy(c->names[slot], name);
        if (c->used > c->peak)
        {
            c->peak = c->used;
        }
        emit(c, OP_STORE);
        emit(c, slot);
        expression(c);
        emit(c, OP_NIL);
        emit(c, OP_STORE);
        emit(c, slot);
        c->used--;
    }
    else if (!strcmp(op, "set"))
    {
        if (c->scan.t.kind != WORD || (index = local(c, c->scan.t.word)) < 0)
        {
            c->failed = 1;
            goto done;
        }
        next(&c->scan);
        expression(c);
        emit(c, OP_STORE);
        emit(c, (unsigned)index);
        emit(c, OP_LOAD);
        emit(c, (unsigned)index);
    }
    else if (!strcmp(op, "if"))
    {
        expression(c);
        a = branch(c, OP_JFALSE);
        expression(c);
        b = branch(c, OP_JMP);
        patch(c, a, c->p.length);
        expression(c);
        patch(c, b, c->p.length);
    }
    else if (!strcmp(op, "while"))
    {
        a = c->p.length;
        expression(c);
        b = branch(c, OP_JFALSE);
        expression(c);
        emit(c, OP_POP);
        slot = branch(c, OP_JMP);
        patch(c, slot, a);
        patch(c, b, c->p.length);
        emit(c, OP_NIL);
    }
    else if (!strcmp(op, "do"))
    {
        expression(c);
        emit(c, OP_POP);
        expression(c);
    }
    else if (!strcmp(op, "call"))
    {
        if (c->scan.t.kind != WORD)
        {
            c->failed = 1;
            goto done;
        }
        index = -1;
        for (a = 0; a < c->p.count; a++)
        {
            if (!strcmp(c->defs[a].name, c->scan.t.word))
            {
                index = (int)a;
            }
        }
        next(&c->scan);
        argc = 0;
        while (c->scan.t.kind != RIGHT && !c->failed)
        {
            expression(c);
            if (++argc > CVM_LOCALS)
            {
                c->failed = 1;
            }
        }
        if (index < 0 || argc != c->defs[index].arity)
        {
            c->failed = 1;
        }
        else
        {
            emit(c, OP_CALL);
            emit(c, (unsigned)index);
            emit(c, argc);
        }
    }
    else if (!strcmp(op, "host"))
    {
        if (c->scan.t.kind != NUMBER || c->scan.t.number < 0 ||
            c->scan.t.number > 255)
        {
            c->failed = 1;
            goto done;
        }
        a = (unsigned)c->scan.t.number;
        next(&c->scan);
        expression(c);
        emit(c, OP_HOST);
        emit(c, a);
    }
    else if (!strcmp(op, "gc"))
    {
        emit(c, OP_GC);
        emit(c, OP_NIL);
    }
    else if (!strcmp(op, "first") || !strcmp(op, "second"))
    {
        expression(c);
        emit(c, !strcmp(op, "first") ? OP_FIRST : OP_SECOND);
    }
    else
    {
        unsigned code = 0;
        if (!strcmp(op, "+"))
        {
            code = OP_ADD;
        }
        else if (!strcmp(op, "-"))
        {
            code = OP_SUB;
        }
        else if (!strcmp(op, "*"))
        {
            code = OP_MUL;
        }
        else if (!strcmp(op, "/"))
        {
            code = OP_DIV;
        }
        else if (!strcmp(op, "=="))
        {
            code = OP_EQ;
        }
        else if (!strcmp(op, "<"))
        {
            code = OP_LT;
        }
        else if (!strcmp(op, "pair"))
        {
            code = OP_PAIR;
        }
        if (!code)
        {
            c->failed = 1;
            goto done;
        }
        expression(c);
        expression(c);
        emit(c, code);
    }
    take(c, RIGHT);
done:
    c->depth--;
}

enum cvm_status cvm_compile(const char *source, size_t length,
                            struct cvm_program *out)
{
    struct cvm_compiler c = {0};
    unsigned f;
    if (!source || !out || length > 4096)
    {
        return CVM_SYNTAX;
    }
    c.scan.s = source;
    c.scan.n = length;
    next(&c.scan);
    while (c.scan.t.kind != END && !c.failed)
    {
        struct cvm_definition *d;
        unsigned nesting = 0;
        if (c.p.count >= CVM_FUNCTIONS)
        {
            return CVM_SYNTAX;
        }
        d = &c.defs[c.p.count];
        if (!take(&c, LEFT) || !named(&c.scan.t, "fn"))
        {
            return CVM_SYNTAX;
        }
        next(&c.scan);
        if (c.scan.t.kind != WORD || !ident(c.scan.t.word))
        {
            return CVM_SYNTAX;
        }
        strcpy(d->name, c.scan.t.word);
        for (f = 0; f < c.p.count; f++)
        {
            if (!strcmp(c.defs[f].name, d->name))
            {
                return CVM_SYNTAX;
            }
        }
        next(&c.scan);
        if (!take(&c, LEFT))
        {
            return CVM_SYNTAX;
        }
        while (c.scan.t.kind != RIGHT)
        {
            if (c.scan.t.kind != WORD || !ident(c.scan.t.word) ||
                d->arity >= CVM_LOCALS)
            {
                return CVM_SYNTAX;
            }
            for (f = 0; f < d->arity; f++)
            {
                if (!strcmp(d->params[f], c.scan.t.word))
                {
                    return CVM_SYNTAX;
                }
            }
            strcpy(d->params[d->arity++], c.scan.t.word);
            next(&c.scan);
        }
        next(&c.scan);
        d->start = c.scan.t.begin;
        if (c.scan.t.kind == LEFT)
        {
            do
            {
                if (c.scan.t.kind == LEFT)
                {
                    if (++nesting > 64)
                    {
                        return CVM_SYNTAX;
                    }
                }
                else if (c.scan.t.kind == RIGHT)
                {
                    nesting--;
                }
                else if (c.scan.t.kind == END || c.scan.t.kind == BAD)
                {
                    return CVM_SYNTAX;
                }
                d->end = c.scan.t.end;
                next(&c.scan);
            } while (nesting);
        }
        else if (c.scan.t.kind == WORD || c.scan.t.kind == NUMBER)
        {
            d->end = c.scan.t.end;
            next(&c.scan);
        }
        else
        {
            return CVM_SYNTAX;
        }
        if (!take(&c, RIGHT))
        {
            return CVM_SYNTAX;
        }
        c.p.count++;
    }
    if (c.failed || !c.p.count || strcmp(c.defs[0].name, "main") ||
        c.defs[0].arity)
    {
        return CVM_SYNTAX;
    }
    for (f = 0; f < c.p.count; f++)
    {
        struct cvm_function *fn = &c.p.functions[f];
        struct cvm_definition *d = &c.defs[f];
        fn->entry = c.p.length;
        fn->arity = d->arity;
        c.used = d->arity;
        c.peak = c.used;
        c.depth = 0;
        for (unsigned i = 0; i < c.used; i++)
        {
            strcpy(c.names[i], d->params[i]);
        }
        c.scan.pos = d->start;
        c.scan.n = d->end;
        next(&c.scan);
        expression(&c);
        if (c.failed || c.scan.t.kind != END)
        {
            return CVM_SYNTAX;
        }
        emit(&c, OP_RET);
        fn->length = (uint16_t)(c.p.length - fn->entry);
        fn->locals = (uint8_t)c.peak;
    }
    if (c.failed || cvm_verify(&c.p) != CVM_OK)
    {
        return CVM_SYNTAX;
    }
    *out = c.p;
    return CVM_OK;
}
