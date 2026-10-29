#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "CS_CHECK:%d:%s\n", __LINE__, #x); \
            return 41;                                         \
        }                                                      \
    } while (0)
static uint32_t random_state = 913;

static unsigned rnd(void)
{
    random_state = random_state * 1664525u + 1013904223u;
    return random_state;
}

static struct cs_matrix unit(void)
{
    struct cs_matrix m = {{{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}}};
    return m;
}

static int same(const struct cs_matrix *a, const struct cs_matrix *b)
{
    for (unsigned i = 0; i < 3; i++)
    {
        for (unsigned j = 0; j < 4; j++)
        {
            if (a->m[i][j] != b->m[i][j])
            {
                return 0;
            }
        }
    }
    return 1;
}

static int scene(int dump)
{
    struct cs_scene s, before, other;
    struct cs_handle h[CS_N], bad;
    struct cs_matrix m = unit(), w, old;
    unsigned work;
    cs_init(&s);
    cs_init(&other);
    m.m[0][0] = 0;
    m.m[0][1] = -1;
    m.m[1][0] = 1;
    m.m[1][1] = 0;
    m.m[0][3] = 10;
    CHECK(cs_add(&s, NULL, &m, &h[0]) == CS_OK);
    m = unit();
    m.m[0][3] = 3;
    CHECK(cs_add(&s, &h[0], &m, &h[1]) == CS_OK);
    CHECK(cs_world(&s, h[1], &w, &work) == CS_OK && work == 2 &&
          w.m[0][3] == 10 && w.m[1][3] == 3);
    CHECK(cs_world(&s, h[1], &w, &work) == CS_OK && work == 0);
    m = unit();
    m.m[0][3] = 20;
    CHECK(cs_set(&s, h[0], &m) == CS_OK);
    CHECK(cs_world(&s, h[1], &w, &work) == CS_OK && w.m[0][3] == 23 &&
          work == 2);
    m = unit();
    m.m[1][3] = 7;
    CHECK(cs_add(&s, NULL, &m, &h[2]) == CS_OK);
    old = w;
    CHECK(cs_reparent(&s, h[1], &h[2], 1) == CS_OK);
    CHECK(cs_world(&s, h[1], &w, &work) == CS_OK && same(&old, &w));
    memcpy(&before, &s, sizeof s);
    CHECK(cs_reparent(&s, h[2], &h[1], 0) == CS_INVALID &&
          memcmp(&before, &s, sizeof s) == 0);
    CHECK(cs_remove(&s, h[2]) == CS_BUSY);
    CHECK(cs_world(&other, h[1], &w, &work) == CS_STALE);
    m = unit();
    m.m[0][0] = 2;
    CHECK(cs_set(&s, h[0], &m) == CS_OK);
    memcpy(&before, &s, sizeof s);
    CHECK(cs_reparent(&s, h[1], &h[0], 1) == CS_RANGE &&
          memcmp(&before, &s, sizeof s) == 0);
    m.m[0][0] = 0;
    CHECK(cs_set(&s, h[0], &m) == CS_OK);
    memcpy(&before, &s, sizeof s);
    CHECK(cs_reparent(&s, h[1], &h[0], 1) == CS_RANGE &&
          memcmp(&before, &s, sizeof s) == 0);
    m.m[0][0] = 2;
    CHECK(cs_set(&s, h[0], &m) == CS_OK);
    old = s.node[h[1].slot].local;
    CHECK(cs_reparent(&s, h[1], &h[0], 0) == CS_OK);
    CHECK(same(&old, &s.node[h[1].slot].local));
    CHECK(cs_world(&s, h[1], &w, &work) == CS_OK && w.m[0][3] == 46 &&
          w.m[1][3] == -7);
    CHECK(cs_remove(&s, h[1]) == CS_OK);
    bad = h[1];
    m = unit();
    CHECK(cs_add(&s, NULL, &m, &h[1]) == CS_OK);
    CHECK(cs_world(&s, bad, &w, &work) == CS_STALE);
    /* Explicit seeded finite boundary, not a forged handle: issueMAX once then
     * retire. */
    CHECK(cs_remove(&s, h[1]) == CS_OK);
    s.node[h[1].slot].generation = UINT32_MAX - 1;
    CHECK(cs_add(&s, NULL, &m, &h[1]) == CS_OK &&
          h[1].generation == UINT32_MAX);
    CHECK(cs_remove(&s, h[1]) == CS_OK);
    CHECK(cs_add(&s, NULL, &m, &bad) == CS_OK && bad.slot != h[1].slot);
    struct cs_scene overflow;
    struct cs_handle chain[6];
    cs_init(&overflow);
    m = unit();
    m.m[0][0] = 16;
    for (unsigned i = 0; i < 6; i++)
    {
        CHECK(cs_add(&overflow, i ? &chain[i - 1] : NULL, &m, &chain[i]) ==
              CS_OK);
    }
    memcpy(&before, &overflow, sizeof before);
    w = unit();
    old = w;
    work = 77;
    CHECK(cs_world(&overflow, chain[5], &w, &work) == CS_RANGE &&
          same(&w, &old) && work == 77 &&
          memcmp(&before, &overflow, sizeof before) == 0);
    /* Independent matrix oracle consumes actual source state and every queried
     * result. */
    struct cs_scene many;
    struct cs_handle handles[CS_N];
    cs_init(&many);
    for (unsigned i = 0; i < CS_N; i++)
    {
        m = unit();
        m.m[0][3] = (int32_t)i - 8;
        CHECK(cs_add(&many, i ? &handles[rnd() % i] : NULL, &m, &handles[i]) ==
              CS_OK);
    }
    unsigned computed = 0, cached = 0;
    for (unsigned step = 0; step < 256; step++)
    {
        unsigned at = rnd() % CS_N;
        m = unit();
        unsigned axis = rnd() % 3;
        m.m[axis][axis] = -1;
        m.m[0][3] = (int32_t)(rnd() % 9) - 4;
        m.m[1][3] = (int32_t)(rnd() % 9) - 4;
        m.m[2][3] = (int32_t)(rnd() % 9) - 4;
        if (step % 3 == 0)
        {
            m.m[0][1] = 1;
        } /* shear is admitted here, deliberately beyond glTFTRS. */
        CHECK(cs_set(&many, handles[at], &m) == CS_OK);
        if (dump)
        {
            printf("S %u\n", step);
            for (unsigned i = 0; i < CS_N; i++)
            {
                printf("L %u %u", i, many.node[i].parent);
                for (unsigned a = 0; a < 3; a++)
                {
                    for (unsigned b = 0; b < 4; b++)
                    {
                        printf(" %d", many.node[i].local.m[a][b]);
                    }
                }
                puts("");
            }
        }
        for (unsigned i = 0; i < CS_N; i++)
        {
            CHECK(cs_world(&many, handles[i], &w, &work) == CS_OK);
            computed += work;
            if (dump)
            {
                printf("W %u", i);
                for (unsigned a = 0; a < 3; a++)
                {
                    for (unsigned b = 0; b < 4; b++)
                    {
                        printf(" %d", w.m[a][b]);
                    }
                }
                printf(" %u\n", work);
            }
            CHECK(cs_world(&many, handles[i], &old, &work) == CS_OK &&
                  work == 0 && same(&w, &old));
            cached++;
        }
    }
    printf("COST scene %u %u %zu %zu\n", computed, cached,
           sizeof(struct cs_scene), sizeof(struct cs_matrix) * CS_N);
    return 0;
}

static struct nv_polygon box(int x0, int y0, int y1, int x1, int z0, int z1)
{
    struct nv_polygon p = {4, {{x0, y0}, {x1, z0}, {x1, z1}, {x0, y1}}};
    return p;
}

static struct nv_point transform(struct nv_point p, unsigned t)
{
    if (t == 1)
    {
        int32_t x = p.x;
        p.x = -p.y;
        p.y = x;
    }
    if (t == 2)
    {
        p.x = -p.x;
    }
    if (t == 3)
    {
        p.x *= 2;
        p.y *= 2;
    }
    return p;
}

static int nav(int dump)
{
    struct nv_polygon c[8];
    struct nv_scratch scratch;
    struct nv_path out, saved;
    struct nv_point a = {1, 1}, b = {7, 7};
    c[0] = box(0, 0, 4, 4, 0, 4);
    c[1] = box(4, 0, 4, 8, 0, 4);
    c[2] = box(4, 4, 8, 8, 4, 8);
    CHECK(nv_path(c, 3, a, b, &scratch, 4, 10, &out) == CS_OK);
    CHECK(out.n == 3 && out.p[1].x == 4 && out.p[1].y == 4);
    a.y = 3;
    CHECK(nv_path(c, 3, a, b, &scratch, 4, 10, &out) == CS_OK && out.n == 3 &&
          out.p[1].x == 4 && out.p[1].y == 4);
    memcpy(&saved, &out, sizeof out);
    CHECK(nv_path(c, 3, a, b, &scratch, 3, 10, &out) == CS_FULL &&
          memcmp(&out, &saved, sizeof out) == 0);
    CHECK(nv_path(c, 3, a, b, &scratch, 4, 2, &out) == CS_FULL &&
          memcmp(&out, &saved, sizeof out) == 0);
    struct nv_session session, other;
    struct nv_ticket t, old;
    nv_init(&session);
    nv_init(&other);
    CHECK(nv_begin(&session, c, 3, a, b, &scratch, 4, 10, &t) == CS_OK);
    old = t;
    CHECK(nv_cancel(&session, t) == CS_OK);
    CHECK(nv_begin(&session, c, 3, a, b, &scratch, 4, 10, &t) == CS_OK);
    CHECK(nv_copy(&session, old, &out) == CS_STALE);
    CHECK(nv_copy(&other, t, &out) == CS_STALE);
    memset(&scratch, 0, sizeof scratch);
    CHECK(nv_copy(&session, t, &out) == CS_OK && out.n == 3);
    CHECK(nv_begin(&session, c, 3, a, b, &scratch, 4, 10, &t) == CS_BUSY);
    old = t;
    CHECK(nv_begin(&session, NULL, 0, a, b, NULL, 0, 0, &t) == CS_BUSY);
    CHECK(t.owner == old.owner && t.generation == old.generation &&
          session.active);
    CHECK(nv_begin(&session, NULL, 0, a, b, NULL, 0, 0, NULL) == CS_INVALID);
    CHECK(nv_cancel(&session, t) == CS_OK);
    session.generation = UINT32_MAX - 1;
    CHECK(nv_begin(&session, c, 3, a, b, &scratch, 4, 10, &t) == CS_OK &&
          t.generation == UINT32_MAX);
    CHECK(nv_cancel(&session, t) == CS_OK);
    CHECK(nv_begin(&session, c, 3, a, b, &scratch, 4, 10, &t) == CS_FULL);
    old = t;
    CHECK(nv_begin(&session, NULL, 0, a, b, NULL, 0, 0, &t) == CS_FULL);
    CHECK(t.owner == old.owner && t.generation == old.generation &&
          !session.active);
    c[1] = c[0];
    CHECK(nv_path(c, 3, a, b, &scratch, 4, 10, &out) == CS_INVALID);
    /* Two triangles, a portal endpoint start, and coincident start/end. */
    c[0] = (struct nv_polygon){3, {{0, 0}, {4, 0}, {0, 4}}};
    c[1] = (struct nv_polygon){3, {{4, 0}, {4, 4}, {0, 4}}};
    a = (struct nv_point){0, 4};
    b = (struct nv_point){4, 4};
    CHECK(nv_path(c, 2, a, b, &scratch, 3, 10, &out) == CS_OK && out.n == 2);
    CHECK(nv_path(c, 1, a, a, &scratch, 2, 1, &out) == CS_OK && out.n == 1);
    random_state = 1481;
    for (unsigned seed = 0; seed < 160; seed++)
    {
        unsigned n = 1 + rnd() % 8;
        int center[9];
        for (unsigned i = 0; i <= n; i++)
        {
            center[i] = (int)(rnd() % 17) - 8;
        }
        struct nv_polygon original[8];
        for (unsigned i = 0; i < n; i++)
        {
            original[i] =
                box((int)i * 3, center[i] - 1, center[i] + 1, (int)(i + 1) * 3,
                    center[i + 1] - 1, center[i + 1] + 1);
        }
        struct nv_point first = {0, center[0]},
                        last = {(int32_t)n * 3, center[n]};
        for (unsigned variant = 0; variant < 8; variant++)
        {
            unsigned shape = variant % 4;
            for (unsigned i = 0; i < n; i++)
            {
                c[i] = original[i];
                for (unsigned j = 0; j < 4; j++)
                {
                    c[i].p[j] =
                        transform(original[i].p[shape == 2 ? 3 - j : j], shape);
                }
            }
            a = transform(first, shape);
            b = transform(last, shape);
            if (variant >= 4)
            {
                for (unsigned i = 0; i < n / 2; i++)
                {
                    struct nv_polygon temp = c[i];
                    c[i] = c[n - 1 - i];
                    c[n - 1 - i] = temp;
                }
                struct nv_point temp = a;
                a = b;
                b = temp;
            }
            CHECK(nv_path(c, n, a, b, &scratch, n + 1, 10, &out) == CS_OK &&
                  out.n <= n + 1);
            if (dump)
            {
                printf("N %u %u %u %d %d %d %d\n", seed, variant, n, a.x, a.y,
                       b.x, b.y);
                for (unsigned i = 0; i < n; i++)
                {
                    printf("P %u", c[i].n);
                    for (unsigned j = 0; j < c[i].n; j++)
                    {
                        printf(" %d %d", c[i].p[j].x, c[i].p[j].y);
                    }
                    puts("");
                }
                printf("R %u", out.n);
                for (unsigned i = 0; i < out.n; i++)
                {
                    printf(" %d %d", out.p[i].x, out.p[i].y);
                }
                puts("");
            }
            memcpy(&saved, &out, sizeof out);
            CHECK(nv_path(c, n, a, b, &scratch, n, 10, &out) == CS_FULL &&
                  memcmp(&out, &saved, sizeof out) == 0);
        }
    }
    printf("COST nav %zu %zu %zu\n", sizeof(struct nv_scratch),
           sizeof(struct nv_session), sizeof(struct nv_path));
    return 0;
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        return 2;
    }
    unsigned mode = (unsigned)strtoul(argv[1], NULL, 10);
    return mode == 1 ? scene(argc > 2) : mode == 2 ? nav(argc > 2) : 2;
}
