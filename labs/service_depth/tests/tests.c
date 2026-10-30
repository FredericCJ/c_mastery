#include "service_depth.h"
#include <stdio.h>
#include <string.h>
#define CHECK(x)                                               \
    do                                                         \
    {                                                          \
        if (!(x))                                              \
        {                                                      \
            fprintf(stderr, "SD_CHECK:%d:%s\n", __LINE__, #x); \
            return 1;                                          \
        }                                                      \
    } while (0)

/* Runs a sub-check that returns nonzero on its first failed CHECK. */
#define RUN(step)        \
    do                   \
    {                    \
        int rc = (step); \
        if (rc)          \
        {                \
            return rc;   \
        }                \
    } while (0)

/* Test geometry. The expected values below are written out as literals on
 * purpose: they restate the contract and do not depend on the implementation. */
enum
{
    FRAME_COUNT = 16,
    FRAME_BYTES = 64,
    LONG_IMAGE_BYTES = 130
};

/* Validator whose verdict is the int `context` points to. */
static int validate(void *ctx, void *p, size_t n)
{
    (void)p;
    (void)n;
    return *(int *)ctx;
}

/* Validator that also records how often and with what length it was called. */
struct sd_validation
{
    int verdict;
    size_t calls;
    size_t length;
};

static int counted_validate(void *ctx, void *p, size_t n)
{
    struct sd_validation *v = ctx;
    (void)p;
    v->calls++;
    v->length = n;
    return v->verdict;
}

static int all_frames_free(const struct sd_memory *m)
{
    for (unsigned i = 0; i < FRAME_COUNT; i++)
    {
        CHECK(m->refs[i] == 0);
    }
    return 0;
}

/* Failed construction releases only its own frames and never publishes. */
static int memory_load_failures(struct sd_memory *m, const unsigned char *data)
{
    struct sd_image a = {0};
    for (int fail = 0; fail < 3; fail++)
    {
        a.length = 99;
        CHECK(sd_load(m, data, LONG_IMAGE_BYTES, fail, &a) == SD_FULL &&
              a.length == 99);
        RUN(all_frames_free(m));
    }
    return 0;
}

/* Clone, copy-on-write and release keep every reference count balanced. */
static int memory_copy_on_write(struct sd_memory *m, const unsigned char *data)
{
    struct sd_image a = {0};
    struct sd_image b = {0};
    CHECK(sd_load(m, data, LONG_IMAGE_BYTES, -1, &a) == 0 && a.count == 3 &&
          m->bytes[a.frames[2]][1] == 130 && m->bytes[a.frames[2]][2] == 0);
    a.kind[1] = 1;
    struct sd_image before = a;
    CHECK(sd_clone(m, &a, 1, &b) == SD_DENIED &&
          !memcmp(&a, &before, sizeof a));
    a.kind[1] = 0;
    CHECK(sd_clone(m, &a, 2, &b) == SD_INVALID);
    CHECK(sd_clone(m, &a, 1, &b) == 0 && m->refs[a.frames[0]] == 2 &&
          a.cow[0] && b.cow[0]);
    CHECK(sd_write(m, &b, 0, 99, 0) == SD_FULL &&
          m->bytes[a.frames[0]][0] == 1);
    CHECK(sd_write(m, &b, 0, 99, 1) == 0 && b.frames[0] != a.frames[0] &&
          m->bytes[a.frames[0]][0] == 1 && m->bytes[b.frames[0]][0] == 99);
    CHECK(sd_release(m, &b) == 0);
    unsigned old = a.frames[0];
    CHECK(sd_write(m, &a, 0, 88, 0) == 0 && a.frames[0] == old);
    CHECK(sd_release(m, &a) == 0);
    return all_frames_free(m);
}

/* Preserve existing ownership when the final needed frame is unavailable. */
static int memory_exhaustion(struct sd_memory *m)
{
    struct sd_image occupied[4];
    struct sd_image a = {0};
    unsigned char page[256] = {3};
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(sd_load(m, page, sizeof page, -1, &occupied[i]) == SD_OK);
    }
    a.length = 123;
    CHECK(sd_load(m, page, 1, -1, &a) == SD_FULL && a.length == 123);
    for (unsigned i = 0; i < FRAME_COUNT; i++)
    {
        CHECK(m->refs[i] == 1);
    }
    for (unsigned i = 0; i < 4; i++)
    {
        CHECK(sd_release(m, &occupied[i]) == SD_OK);
    }
    return 0;
}

/* A saturated reference count makes clone fail before any parent change. */
static int memory_reference_limit(struct sd_memory *m,
                                  const unsigned char *data)
{
    struct sd_image a = {0};
    struct sd_image b = {0};
    CHECK(sd_load(m, data, LONG_IMAGE_BYTES, -1, &a) == SD_OK);
    /* Controlled reference-count boundary injection; restore before teardown. */
    m->refs[a.frames[2]] = 65535;
    struct sd_image before = a;
    b.length = 321;
    CHECK(sd_clone(m, &a, 1, &b) == SD_FULL && b.length == 321 &&
          !memcmp(&a, &before, sizeof a) && m->refs[a.frames[0]] == 1);
    m->refs[a.frames[2]] = 1;
    CHECK(sd_release(m, &a) == SD_OK && sd_release(m, &a) == SD_INVALID);
    return 0;
}

static int memory(void)
{
    struct sd_memory m = {0};
    unsigned char data[LONG_IMAGE_BYTES];
    for (unsigned i = 0; i < sizeof data; i++)
    {
        data[i] = (unsigned char)(i + 1);
    }
    RUN(memory_load_failures(&m, data));
    RUN(memory_copy_on_write(&m, data));
    RUN(memory_exhaustion(&m));
    RUN(memory_reference_limit(&m, data));
    return 0;
}

static int channel(void)
{
    struct sd_channel q;
    struct sd_cap all = {1, 7};
    struct sd_cap send;
    struct sd_cap recv;
    CHECK(sd_channel_init(&q) == 0);
    CHECK(sd_grant(&q, all, 1, &send) == 0 && sd_grant(&q, all, 2, &recv) == 0);
    CHECK(sd_grant(&q, recv, 3, &all) == SD_DENIED);
    all = (struct sd_cap){1, 7};
    unsigned char data[] = {1, 0, 255};
    unsigned char out[8];
    memset(out, 77, 8);
    size_t n = 99;
    int fail = 0;
    CHECK(sd_send(&q, send, data, 3) == 0);
    /* Directional rights: the send capability cannot receive. */
    CHECK(sd_receive(&q, send, out, 8, &n, validate, &fail) == SD_DENIED &&
          q.count == 1 && n == 99);
    /* A refused copy-out leaves message, length output and destination. */
    fail = 1;
    CHECK(sd_receive(&q, recv, out, 8, &n, validate, &fail) == SD_COPY &&
          q.count == 1 && n == 99 && out[0] == 77);
    fail = 0;
    CHECK(sd_receive(&q, recv, out, 2, &n, validate, &fail) == SD_FULL &&
          q.count == 1);
    CHECK(sd_channel_close(&q, all) == 0);
    CHECK(sd_send(&q, send, data, 3) == SD_CLOSED);
    CHECK(sd_receive(&q, recv, out, 8, &n, validate, &fail) == 0 && n == 3 &&
          !memcmp(out, data, 3));
    CHECK(sd_receive(&q, recv, out, 8, &n, validate, &fail) == SD_CLOSED);
    CHECK(sd_channel_reopen(&q) == 0);
    CHECK(sd_send(&q, send, data, 3) == SD_STALE);
    all = (struct sd_cap){q.epoch, 7};
    CHECK(sd_send(&q, all, NULL, 0) == SD_OK);
    CHECK(sd_send(&q, all, data, 3) == SD_OK);
    CHECK(sd_send(&q, all, data, 3) == SD_FULL);
    /* Zero-length messages still run the validator exactly once. */
    struct sd_validation validation = {1, 0, 99};
    n = 99;
    CHECK(sd_receive(&q, all, NULL, 0, &n, counted_validate, &validation) ==
              SD_COPY &&
          validation.calls == 1 && validation.length == 0 && n == 99 &&
          q.count == 2);
    validation = (struct sd_validation){0, 0, 99};
    CHECK(sd_receive(&q, all, NULL, 0, &n, counted_validate, &validation) ==
              SD_OK &&
          validation.calls == 1 && validation.length == 0 && n == 0 &&
          q.count == 1);
    CHECK(sd_receive(&q, all, out, 8, &n, validate, &fail) == SD_OK && n == 3);
    /* The last epoch value retires the channel instead of wrapping. */
    q.epoch = UINT32_MAX;
    all = (struct sd_cap){UINT32_MAX, 7};
    CHECK(sd_channel_close(&q, all) == 0 && sd_channel_reopen(&q) == SD_FULL &&
          q.retired);
    return 0;
}

/* Handle and description tables are exactly 8 and 4 slots. */
static int files_capacity(struct sd_files *fs)
{
    struct sd_handle held[8];
    struct sd_handle extra;
    CHECK(sd_open(fs, "alpha", &held[0]) == SD_OK);
    for (unsigned i = 1; i < 8; i++)
    {
        CHECK(sd_dup(fs, held[0], &held[i]) == SD_OK);
    }
    extra = (struct sd_handle){99, 99};
    CHECK(sd_dup(fs, held[0], &extra) == SD_FULL && extra.index == 99);
    for (unsigned i = 0; i < 8; i++)
    {
        CHECK(sd_close(fs, held[i]) == SD_OK);
    }
    /* Exhausted generations never return a previously issued incarnation. */
    for (unsigned i = 0; i < 8; i++)
    {
        fs->handles[i].generation = UINT32_MAX;
    }
    CHECK(sd_open(fs, "alpha", &extra) == SD_FULL && extra.index == 99);
    return 0;
}

static int files(void)
{
    static const unsigned char data[] = "0123456789";
    struct sd_entry e[] = {{"alpha", data, 10}, {"empty", NULL, 0}};
    struct sd_files fs;
    struct sd_handle a;
    struct sd_handle b;
    struct sd_handle c;
    int fail = 0;
    unsigned char out[12];
    size_t n;
    CHECK(sd_files_init(&fs, e, 2) == 0);
    /* Two opens have independent offsets; a dup shares one. */
    CHECK(sd_open(&fs, "alpha", &a) == 0 && sd_open(&fs, "alpha", &b) == 0);
    CHECK(sd_read(&fs, a, out, 12, 3, &n, validate, &fail) == 0 && n == 3 &&
          !memcmp(out, "012", 3));
    CHECK(sd_read(&fs, b, out, 12, 4, &n, validate, &fail) == 0 && n == 4 &&
          !memcmp(out, "0123", 4));
    CHECK(sd_dup(&fs, a, &c) == 0);
    CHECK(sd_read(&fs, c, out, 12, 2, &n, validate, &fail) == 0 &&
          !memcmp(out, "34", 2));
    CHECK(sd_read(&fs, a, out, 12, 1, &n, validate, &fail) == 0 &&
          out[0] == '5');
    /* A refused copy-out must not advance the shared offset. */
    fail = 1;
    n = 99;
    CHECK(sd_read(&fs, a, out, 12, 2, &n, validate, &fail) == SD_COPY &&
          n == 99);
    fail = 0;
    CHECK(sd_read(&fs, a, out, 12, 1, &n, validate, &fail) == 0 &&
          out[0] == '6');
    CHECK(sd_seek(&fs, a, 2, -1) == 0);
    CHECK(sd_read(&fs, c, out, 12, 3, &n, validate, &fail) == 0 && n == 1 &&
          out[0] == '9');
    struct sd_validation validation = {0, 0, 99};
    CHECK(sd_read(&fs, a, NULL, 3, 3, &n, counted_validate, &validation) ==
              SD_OK &&
          validation.calls == 1 && validation.length == 0 && n == 0);
    CHECK(sd_seek(&fs, a, 0, -1) == SD_INVALID);
    CHECK(sd_close(&fs, a) == 0 && sd_close(&fs, a) == SD_STALE);
    CHECK(sd_close(&fs, c) == 0 && sd_close(&fs, b) == 0);
    CHECK(sd_open(&fs, "../alpha", &a) == SD_INVALID);
    struct sd_files before = fs;
    struct sd_entry duplicate[] = {e[0], e[0]};
    CHECK(sd_files_init(&fs, duplicate, 2) == SD_INVALID &&
          !memcmp(&fs, &before, sizeof fs));
    CHECK(sd_files_init(&fs, e, 0) == SD_INVALID &&
          !memcmp(&fs, &before, sizeof fs));
    CHECK(sd_open(&fs, "empty", &a) == SD_OK);
    before = fs;
    validation = (struct sd_validation){1, 0, 99};
    n = 99;
    CHECK(sd_read(&fs, a, NULL, 0, 0, &n, counted_validate, &validation) ==
              SD_COPY &&
          validation.calls == 1 && validation.length == 0 && n == 99 &&
          !memcmp(&fs, &before, sizeof fs));
    validation = (struct sd_validation){0, 0, 99};
    CHECK(sd_read(&fs, a, NULL, 0, 0, &n, counted_validate, &validation) ==
              SD_OK &&
          validation.calls == 1 && validation.length == 0 && n == 0);
    CHECK(sd_close(&fs, a) == SD_OK);
    return files_capacity(&fs);
}

/* Injected interruption at each of the four commit steps, then recovery:
 * only stage 4 (committed flush, reply lost) leaves a durable receipt. */
static int journal_fault_stages(void)
{
    for (unsigned fault = 1; fault <= 4; fault++)
    {
        struct sd_journal j = {0};
        int32_t out = 99;
        struct sd_receipt r;
        CHECK(sd_apply(&j, 1, 7, fault, &out) == SD_UNCERTAIN && out == 99);
        CHECK(sd_recover(&j, &r) == 0 && r.last == (fault == 4 ? 1u : 0u) &&
              r.value == (fault == 4 ? 7 : 0));
        CHECK(sd_apply(&j, 1, 7, 0, &out) == 0 && out == 7);
        CHECK(sd_apply(&j, 1, 8, 0, &out) == SD_CONFLICT && out == 7);
        CHECK(sd_apply(&j, 2, -3, 0, &out) == 0 && out == 4);
        CHECK(sd_apply(&j, 1, 7, 0, &out) == SD_STALE);
    }
    return 0;
}

static int journal(void)
{
    RUN(journal_fault_stages());
    struct sd_journal j = {0};
    int32_t out = 0;
    CHECK(sd_apply(&j, 1, INT32_MAX, 0, &out) == 0);
    CHECK(sd_apply(&j, 2, 1, 0, &out) == SD_FULL && out == INT32_MAX);
    j.durable[1][12] ^= 1;
    struct sd_receipt r = {99, 99, 99};
    CHECK(sd_recover(&j, &r) == SD_CORRUPT && r.last == 99);
    /* Alternating slots retain the latest committed receipt at every cut. */
    for (unsigned fault = 1; fault <= 4; fault++)
    {
        j = (struct sd_journal){0};
        CHECK(sd_apply(&j, 1, -10, 0, &out) == SD_OK);
        CHECK(sd_apply(&j, 2, 3, 0, &out) == SD_OK);
        out = 123;
        CHECK(sd_apply(&j, 3, 4, fault, &out) == SD_UNCERTAIN && out == 123);
        CHECK(sd_recover(&j, &r) == SD_OK && r.last == (fault == 4 ? 3u : 2u) &&
              r.value == (fault == 4 ? -3 : -7));
        CHECK(sd_apply(&j, 3, 4, 0, &out) == SD_OK && out == -3);
    }
    return 0;
}

/* Selects one core by name; exit 2 for a usage error, 1 for a failed CHECK. */
int main(int argc, char **argv)
{
    if (argc != 2)
    {
        return 2;
    }
    int status = 2;
    if (!strcmp(argv[1], "memory"))
    {
        status = memory();
    }
    else if (!strcmp(argv[1], "channel"))
    {
        status = channel();
    }
    else if (!strcmp(argv[1], "files"))
    {
        status = files();
    }
    else if (!strcmp(argv[1], "journal"))
    {
        status = journal();
    }
    if (!status)
    {
        puts("PASS service depth");
    }
    return status;
}
