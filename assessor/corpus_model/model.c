#include "model.h"
#include "parsing.h"
#include <limits.h>
#ifndef CM_MUTANT
#define CM_MUTANT 0
#endif
void cm_init(struct cm_model *m)
{
    if (m)
        *m = (struct cm_model){0};
}

static int bound(int64_t n, int32_t *out)
{
    if (n < INT32_MIN || n > INT32_MAX)
        return CM_RANGE;
    *out = (int32_t)n;
    return CM_OK;
}

static int resolve(struct cm_model *m)
{
    unsigned done[CM_NODES] = {0}, left = 0;
    for (unsigned i = 0; i < CM_NODES; ++i)
    {
        struct cm_node *n = &m->nodes[i];
        if (n->kind > CM_LINEAR)
            return CM_INVALID;
        if (n->kind == CM_ABSENT)
        {
            if (n->a || n->b || n->w0 || n->w1 || n->bias || n->value)
                return CM_INVALID;
            done[i] = 1;
        }
        else if (n->kind == CM_INPUT)
        {
            if (n->a || n->b || n->w0 || n->w1)
                return CM_INVALID;
            n->value = n->bias;
            done[i] = 1;
        }
        else
        {
            if (n->a >= CM_NODES || n->b >= CM_NODES)
                return CM_INVALID;
            if (!m->nodes[n->a].kind || !m->nodes[n->b].kind)
                return CM_MISSING;
            ++left;
        }
    }
    while (left)
    {
        unsigned progress = 0;
        for (unsigned i = 0; i < CM_NODES; ++i)
            if (!done[i])
            {
                struct cm_node *n = &m->nodes[i];
                int32_t x, y, z;
                if (!done[n->a] || !done[n->b])
                    continue;
                if (bound((int64_t)m->nodes[n->a].value * n->w0, &x) ||
                    bound((int64_t)m->nodes[n->b].value * n->w1, &y) ||
                    bound((int64_t)x + y, &z) ||
                    bound((int64_t)z + n->bias, &z))
                    return CM_RANGE;
                n->value = CM_MUTANT == 3 ? n->bias : z;
                done[i] = 1;
                --left;
                ++progress;
            }
        if (!progress)
            return CM_MUTANT == 1 ? CM_OK : CM_CYCLE;
    }
    return CM_OK;
}

int cm_valid(const struct cm_model *m)
{
    if (!m)
        return CM_INVALID;
    struct cm_model copy = *m;
    int rc = resolve(&copy);
    if (rc)
        return rc;
    for (unsigned i = 0; i < CM_NODES; ++i)
        if (m->nodes[i].value != copy.nodes[i].value)
            return CM_INVALID;
    return CM_OK;
}

int cm_apply(struct cm_model *m, const struct cm_edit *edits, size_t count)
{
    unsigned seen[CM_NODES] = {0};
    if (!m || (!edits && count))
        return CM_INVALID;
    int rc = cm_valid(m);
    if (rc)
        return rc;
    if (count > CM_NODES)
        return CM_RANGE;
    if (!count)
        return CM_OK;
    if (m->revision == UINT32_MAX)
        return CM_RANGE;
    struct cm_model stage = *m;
    for (size_t i = 0; i < count; ++i)
    {
        const struct cm_edit *e = &edits[i];
        if (e->id >= CM_NODES || (e->kind != CM_INPUT && e->kind != CM_LINEAR))
            return CM_INVALID;
        if (seen[e->id])
            return CM_CONFLICT;
        seen[e->id] = 1;
        if (e->kind == CM_INPUT && (e->a || e->b || e->w0 || e->w1))
            return CM_INVALID;
        stage.nodes[e->id] =
            (struct cm_node){e->kind, e->a, e->b, e->w0, e->w1, e->bias, 0};
    }
    if (CM_MUTANT == 2)
        *m = stage;
    rc = resolve(&stage);
    if (rc)
        return rc;
    ++stage.revision;
    *m = stage;
    return CM_OK;
}

struct cm_reader
{
    const unsigned char *s;
    size_t n, p;
    unsigned csv;
};

static int space(unsigned char c)
{
    return c == ' ' || c == '\t' || c == '\r';
}

static void ws(struct cm_reader *r)
{
    while (r->p < r->n && space(r->s[r->p]))
        ++r->p;
}

static int separator(struct cm_reader *r)
{
    if (r->csv)
    {
        if (r->p == r->n || r->s[r->p] != ',')
            return CM_SYNTAX;
        ++r->p;
    }
    else
    {
        if (r->p == r->n || !space(r->s[r->p]))
            return CM_SYNTAX;
        ws(r);
    }
    return CM_OK;
}

static int number(struct cm_reader *r, int32_t *out)
{
    unsigned negative = 0;
    uint32_t value = 0, limit = INT32_MAX;
    size_t begin;
    if (r->p < r->n && r->s[r->p] == '-')
    {
        negative = 1;
        limit = UINT32_C(2147483648);
        ++r->p;
    }
    begin = r->p;
    while (r->p < r->n && r->s[r->p] >= '0' && r->s[r->p] <= '9')
    {
        unsigned digit = r->s[r->p] - '0';
        if (value > (limit - digit) / 10)
            return CM_RANGE;
        value = value * 10 + digit;
        ++r->p;
    }
    if (begin == r->p)
        return CM_SYNTAX;
    *out = negative
               ? (value == UINT32_C(2147483648) ? INT32_MIN : -(int32_t)value)
               : (int32_t)value;
    return CM_OK;
}

static int word(struct cm_reader *r, const char *s)
{
    size_t p = r->p;
    for (size_t i = 0; s[i]; ++i)
    {
        if (p == r->n || r->s[p] != (unsigned char)s[i])
            return 0;
        ++p;
    }
    if (p < r->n && !(r->csv ? r->s[p] == ',' : space(r->s[p])))
        return 0;
    r->p = p;
    return 1;
}

static int definition(const unsigned char *s, size_t n, unsigned csv,
                      struct cm_edit *out)
{
    struct cm_reader r = {s, n, 0, csv};
    struct cm_edit e = {0};
    int32_t fields[7] = {0};
    unsigned count;
    if (!csv)
        ws(&r);
    if (word(&r, csv ? "I" : "input"))
    {
        e.kind = CM_INPUT;
        count = 2;
    }
    else if (word(&r, csv ? "L" : "linear"))
    {
        e.kind = CM_LINEAR;
        count = 6;
    }
    else
        return CM_SYNTAX;
    for (unsigned i = 0; i < count; ++i)
    {
        int rc = separator(&r);
        if (rc)
            return rc;
        rc = number(&r, &fields[i]);
        if (rc)
            return rc;
    }
    if (!csv)
        ws(&r);
    if (r.p != r.n && !(CM_MUTANT == 6 && csv))
        return CM_SYNTAX;
    if (fields[0] < 0 || fields[0] >= CM_NODES)
        return CM_RANGE;
    e.id = (unsigned)fields[0];
    if (e.kind == CM_INPUT)
        e.bias = fields[1];
    else
    {
        if (fields[1] < 0 || fields[1] >= CM_NODES || fields[3] < 0 ||
            fields[3] >= CM_NODES)
            return CM_RANGE;
        e.a = (unsigned)fields[1];
        e.w0 = fields[2];
        e.b = (unsigned)fields[3];
        e.w1 = fields[4];
        e.bias = fields[5];
    }
    *out = e;
    return CM_OK;
}

int cm_load(struct cm_model *m, const unsigned char *s, size_t n, unsigned csv)
{
    struct cm_edit edits[CM_NODES];
    size_t count = 0, begin = 0;
    if (!m || (!s && n) || csv > 1)
        return CM_INVALID;
    if (n > 1024)
        return CM_RANGE;
    for (size_t p = 0; p <= n; ++p)
        if (p == n || s[p] == '\n')
        {
            if (p != begin)
            {
                if (count == CM_NODES)
                    return CM_RANGE;
                int rc = definition(s + begin, p - begin, csv, &edits[count]);
                if (rc)
                    return rc;
                ++count;
            }
            begin = p + 1;
        }
    return cm_apply(m, edits, count);
}

int cm_get(const struct cm_model *m, unsigned id, int32_t *out)
{
    if (!out || id >= CM_NODES)
        return CM_INVALID;
    int rc = cm_valid(m);
    if (rc)
        return rc;
    if (!m->nodes[id].kind)
        return CM_MISSING;
    *out = m->nodes[id].value;
    return CM_OK;
}

static size_t decimal(int32_t value, char *s)
{
    char reverse[10];
    size_t n = 0, p = 0;
    uint32_t magnitude;
    if (value < 0)
    {
        s[p++] = '-';
        magnitude = (uint32_t)(-(int64_t)value);
    }
    else
        magnitude = (uint32_t)value;
    do
    {
        reverse[n++] = (char)('0' + magnitude % 10);
        magnitude /= 10;
    } while (magnitude);
    while (n)
        s[p++] = reverse[--n];
    return p;
}

int cm_eval(const struct cm_model *m, const unsigned char *s, size_t n,
            int32_t *out)
{
    unsigned char expanded[256];
    size_t used = 0;
    if (!out || (!s && n))
        return CM_INVALID;
    int rc = cm_valid(m);
    if (rc)
        return rc;
    if (n > 256)
        return CM_RANGE;
    for (size_t p = 0; p < n;)
    {
        if (s[p] != '$')
        {
            if (used == sizeof expanded)
                return CM_RANGE;
            expanded[used++] = s[p++];
            continue;
        }
        ++p;
        unsigned id = 0;
        size_t begin = p;
        while (p < n && s[p] >= '0' && s[p] <= '9')
        {
            unsigned d = s[p++] - '0';
            if (id > (15u - d) / 10u || d > 15u)
                return CM_RANGE;
            id = id * 10 + d;
        }
        if (p == begin)
            return CM_SYNTAX;
        int32_t value;
        rc = cm_get(m, id, &value);
        if (rc)
            return rc;
        char text[18];
        size_t len = 0;
        if (CM_MUTANT != 4)
            text[len++] = '(';
        if (value == INT32_MIN)
        {
            const char *minimum = "-2147483647-1";
            for (size_t i = 0; minimum[i]; ++i)
                text[len++] = minimum[i];
        }
        else
            len += decimal(value, text + len);
        if (CM_MUTANT != 4)
            text[len++] = ')';
        if (len > sizeof expanded - used)
            return CM_RANGE;
        for (size_t i = 0; i < len; ++i)
            expanded[used++] = (unsigned char)text[i];
    }
    struct pd_tokens tokens;
    struct pd_tree tree;
    struct pd_diagnostic diagnostic;
    int32_t result;
    rc = pd_lex(expanded, used, &tokens, &diagnostic);
    if (!rc)
        rc = pd_pratt(&tokens, 1, &tree, &diagnostic);
    if (!rc)
        rc = pd_evaluate(&tree, &result);
    if (rc)
        return rc == PD_RANGE ? CM_RANGE : CM_SYNTAX;
    *out = result;
    return CM_OK;
}

void cm_session_init(struct cm_session *s)
{
    if (s)
        *s = (struct cm_session){0};
}

static void reply(struct cm_session *s, const char *prefix, int32_t value,
                  unsigned numeric)
{
    size_t n = 0;
    while (prefix[n])
    {
        s->reply[n] = prefix[n];
        ++n;
    }
    if (numeric)
        n += decimal(value, s->reply + n);
    s->reply[n++] = '\n';
    s->reply_length = n;
    s->sent = 0;
}

static int execute_line(struct cm_session *s, const unsigned char *line,
                        size_t n)
{
    if (!s || (!line && n))
        return CM_INVALID;
    if (s->reply_length)
        return CM_BUSY;
    if (s->closed)
        return CM_CLOSED;
    if (n > 256)
    {
        reply(s, "ERR ", CM_RANGE, 1);
        return CM_OK;
    }
    for (size_t i = 0; i < n; ++i)
        if (line[i] == '\n')
        {
            reply(s, "ERR ", CM_SYNTAX, 1);
            return CM_OK;
        }
    struct cm_reader r = {line, n, 0, 0};
    ws(&r);
    int rc = CM_OK;
    int32_t value = 0;
    unsigned query = 0;
    if (word(&r, "exit"))
    {
        ws(&r);
        if (r.p != n)
            rc = CM_SYNTAX;
        else
        {
            s->closed = 1;
            reply(s, "BYE", 0, 0);
            return CM_OK;
        }
    }
    else if (word(&r, "eval"))
    {
        rc = separator(&r);
        if (!rc)
            rc = cm_eval(&s->model, line + r.p, n - r.p, &value);
        query = 1;
    }
    else if (word(&r, "get") || word(&r, "set"))
    {
        unsigned set = line[r.p - 1] == 't' && r.p >= 3 && line[r.p - 3] == 's';
        int32_t id = 0;
        rc = separator(&r);
        if (!rc)
            rc = number(&r, &id);
        if (!rc && (id < 0 || id >= CM_NODES))
            rc = CM_RANGE;
        if (set)
        {
            if (!rc)
                rc = separator(&r);
            if (!rc)
                rc = number(&r, &value);
            if (!rc && s->model.nodes[id].kind != CM_INPUT)
                rc = CM_MISSING;
        }
        else
            query = 1;
        ws(&r);
        if (!rc && r.p != n)
            rc = CM_SYNTAX;
        if (!rc)
        {
            if (set)
            {
                struct cm_edit e = {(unsigned)id, CM_INPUT, 0, 0, 0, 0, value};
                rc = cm_apply(&s->model, &e, 1);
            }
            else
                rc = cm_get(&s->model, (unsigned)id, &value);
        }
    }
    else
        rc = cm_load(&s->model, line, n, 0);
    if (rc)
        reply(s, "ERR ", rc, 1);
    else if (query)
        reply(s, "VALUE ", value, 1);
    else
        reply(s, "OK", 0, 0);
    return CM_OK;
}

int cm_step(struct cm_session *s, const unsigned char *line, size_t n)
{
    if (!s || (!line && n))
        return CM_INVALID;
    if (s->used || s->discarding)
        return CM_BUSY;
    return execute_line(s, line, n);
}

int cm_feed(struct cm_session *s, const unsigned char *bytes, size_t n,
            size_t *consumed)
{
    if (!s || !consumed || (!bytes && n))
        return CM_INVALID;
    *consumed = 0;
    if (s->reply_length)
        return CM_BUSY;
    if (s->closed)
        return CM_CLOSED;
    for (size_t i = 0; i < n; ++i)
    {
        unsigned char c = bytes[i];
        *consumed = i + 1;
        if (c == '\n')
        {
            if (s->discarding)
                reply(s, "ERR ", CM_RANGE, 1);
            else
                (void)execute_line(s, s->line, s->used);
            s->used = 0;
            s->discarding = 0;
            return CM_OK;
        }
        if (s->used == sizeof s->line)
            s->discarding = 1;
        else if (!s->discarding)
            s->line[s->used++] = c;
    }
    return CM_PENDING;
}

int cm_finish(struct cm_session *s)
{
    if (!s)
        return CM_INVALID;
    if (s->reply_length)
        return CM_BUSY;
    if (s->closed)
        return CM_CLOSED;
    if (s->discarding)
        reply(s, "ERR ", CM_RANGE, 1);
    else if (s->used)
        (void)execute_line(s, s->line, s->used);
    s->used = 0;
    s->discarding = 0;
    s->closed = 1;
    return CM_OK;
}

int cm_drain(struct cm_session *s, cm_write_fn writer, void *context)
{
    if (!s || !writer)
        return CM_INVALID;
    if (!s->reply_length)
        return CM_OK;
    if (CM_MUTANT == 5)
        s->sent = 0;
    size_t remaining = s->reply_length - s->sent;
    size_t accepted = writer(context, s->reply + s->sent, remaining);
    if (accepted > remaining)
        return CM_INVALID;
    if (!accepted)
        return CM_IO;
    s->sent += accepted;
    if (s->sent < s->reply_length)
        return CM_PENDING;
    s->sent = 0;
    s->reply_length = 0;
    return CM_OK;
}
