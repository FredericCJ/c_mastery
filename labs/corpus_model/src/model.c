#include "model.h"

void cm_init(struct cm_model *m)
{
    if (m)
        *m = (struct cm_model){0};
}

int cm_valid(const struct cm_model *m)
{
    (void)m;
    return CM_INVALID;
}

int cm_apply(struct cm_model *m, const struct cm_edit *e, size_t n)
{
    (void)m;
    (void)e;
    (void)n;
    return CM_INVALID;
}

int cm_load(struct cm_model *m, const unsigned char *s, size_t n, unsigned c)
{
    (void)m;
    (void)s;
    (void)n;
    (void)c;
    return CM_INVALID;
}

int cm_get(const struct cm_model *m, unsigned id, int32_t *v)
{
    (void)m;
    (void)id;
    (void)v;
    return CM_INVALID;
}

int cm_eval(const struct cm_model *m, const unsigned char *s, size_t n,
            int32_t *v)
{
    (void)m;
    (void)s;
    (void)n;
    (void)v;
    return CM_INVALID;
}

void cm_session_init(struct cm_session *s)
{
    if (s)
        *s = (struct cm_session){0};
}

int cm_step(struct cm_session *s, const unsigned char *p, size_t n)
{
    (void)s;
    (void)p;
    (void)n;
    return CM_INVALID;
}

int cm_feed(struct cm_session *s, const unsigned char *p, size_t n,
            size_t *used)
{
    (void)s;
    (void)p;
    (void)n;
    (void)used;
    return CM_INVALID;
}

int cm_finish(struct cm_session *s)
{
    (void)s;
    return CM_INVALID;
}

int cm_drain(struct cm_session *s, cm_write_fn w, void *c)
{
    (void)s;
    (void)w;
    (void)c;
    return CM_INVALID;
}
