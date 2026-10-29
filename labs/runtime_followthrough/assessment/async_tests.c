#define _POSIX_C_SOURCE 200809L
#include "async_store.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                \
    do                                                          \
    {                                                           \
        if (!(x))                                               \
        {                                                       \
            printf("VA_CHECK %u %s\n", (unsigned)__LINE__, #x); \
            return 1;                                           \
        }                                                       \
    } while (0)

struct vg_file
{
    int fd;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    unsigned flushes, gate, entered, released, lose_reply;
};

static int transfer(struct vg_file *f, uint32_t page, void *buffer, int writing)
{
    if (page >= STORE_IMAGE_PAGES)
        return -1;
    unsigned char *p = buffer;
    size_t n = STORE_PAGE_SIZE;
    off_t pos = (off_t)page * STORE_PAGE_SIZE;
    while (n)
    {
        ssize_t k =
            writing ? pwrite(f->fd, p, n, pos) : pread(f->fd, p, n, pos);
        if (k < 0 && errno == EINTR)
            continue;
        if (k <= 0)
            return -1;
        p += k;
        n -= (size_t)k;
        pos += k;
    }
    return 0;
}

static int rd(void *p, uint32_t page, void *b)
{
    return transfer(p, page, b, 0);
}

static int wr(void *p, uint32_t page, const void *b)
{
    return transfer(p, page, (void *)b, 1);
}

static int flush(void *p)
{
    struct vg_file *f = p;
    ++f->flushes;
    if (f->gate && f->flushes == f->gate)
    {
        pthread_mutex_lock(&f->mutex);
        f->entered = 1;
        pthread_cond_broadcast(&f->condition);
        while (!f->released)
            pthread_cond_wait(&f->condition, &f->mutex);
        pthread_mutex_unlock(&f->mutex);
    }
    if (fsync(f->fd))
        return -1;
    return f->lose_reply && f->flushes == f->gate ? -1 : 0;
}

static store_io io(struct vg_file *f)
{
    return (store_io){f, rd, wr, flush, STORE_IMAGE_PAGES};
}

static int wait_gate(struct vg_file *f)
{
    struct timespec t;
    if (clock_gettime(CLOCK_REALTIME, &t))
        return 0;
    t.tv_sec += 3;
    pthread_mutex_lock(&f->mutex);
    while (!f->entered)
    {
        int r = pthread_cond_timedwait(&f->condition, &f->mutex, &t);
        if (r)
        {
            pthread_mutex_unlock(&f->mutex);
            return 0;
        }
    }
    pthread_mutex_unlock(&f->mutex);
    return 1;
}

static void release(struct vg_file *f)
{
    pthread_mutex_lock(&f->mutex);
    f->released = 1;
    pthread_cond_broadcast(&f->condition);
    pthread_mutex_unlock(&f->mutex);
}

static int finish(struct vg_va *a)
{
    for (unsigned i = 0; i < 3000; i++)
    {
        int r = va_poll(a);
        if (r != VA_BUSY)
            return r;
        struct timespec t = {0, 1000000};
        nanosleep(&t, 0);
    }
    return VA_BUSY;
}

static struct cvm_program image(void)
{
    struct cvm_program p = {0};
    const uint8_t b[] = {OP_INT, 5, 0, 0, 0, OP_HOST, 3, OP_RET};
    memcpy(p.code, b, sizeof b);
    p.count = 1;
    p.length = sizeof b;
    p.functions[0] = (struct cvm_function){0, sizeof b, 0, 0};
    return p;
}

static store database;
static struct vg_vs session;
static struct vv_work scratch;

static int scenario(unsigned cancel, unsigned lost)
{
    FILE *stream = tmpfile();
    CHECK(stream);
    struct vg_file f = {.fd = fileno(stream)};
    CHECK(!pthread_mutex_init(&f.mutex, 0) &&
          !pthread_cond_init(&f.condition, 0));
    CHECK(store_format(io(&f)) == STORE_OK &&
          store_open(&database, io(&f)) == STORE_OK);
    struct vg_vh h = {&database, 14, VH_VERSIONED5, 0, 1};
    int32_t value = 91;
    CHECK(vh_host(&h, 1, 10, &value) == STORE_OK &&
          store_checkpoint(&database) == STORE_OK);
    f.flushes = 0;
    f.gate = 3;
    f.lose_reply = lost;
    session = (struct vg_vs){0};
    struct cvm_program p = image();
    CHECK(cvm_verify(&p) == CVM_OK &&
          vs_start(&session, &p, VS_CHECKED_V1, 3, 0, 1, 8, &scratch) ==
              VS_RUNNING);
    struct cvm_value out = cvm_int(91);
    CHECK(vs_step(&session, &out) == VS_RUNNING &&
          vs_step(&session, &out) == VS_PENDING);
    struct vg_va task;
    CHECK(va_init(&task) == VA_READY);
    CHECK(va_submit(&task, &session, &h) == VA_BUSY);
    CHECK(wait_gate(&f));
    int observed = va_poll(&task);
    CHECK(va_destroy(&task) == VA_STATE);
    CHECK(va_submit(&task, &session, &h) == VA_STATE);
    CHECK(vs_start(&session, &p, VS_CHECKED_V1, 3, 0, 1, 8, &scratch) ==
          VS_STATE);
    CHECK(vs_step(&session, &out) == VS_STATE && out.integer == 91);
    /* Another independent guest runs while actual file-backed commit is pending.
     */
    struct cvm_cvm other;
    struct cvm_program simple = {0};
    simple.count = 1;
    simple.length = 6;
    simple.functions[0] = (struct cvm_function){0, 6, 0, 0};
    simple.code[0] = OP_INT;
    simple.code[1] = 47;
    simple.code[5] = OP_RET;
    cvm_init(&other, 2, 0, 0);
    struct cvm_value result;
    CHECK(cvm_run(&other, &simple, &result) == CVM_OK && result.integer == 47);
    if (cancel)
        CHECK(va_cancel(&task) == VA_BUSY);
    release(&f);
    int status = finish(&task);
    /* Contain a lying completion specimen before reporting its semantic failure.
     * Normal adapter already joined; this is harness cleanup, not acceptance. */
    if (task.active && !task.joined)
    {
        CHECK(!pthread_join(task.worker, 0));
        task.active = 0;
        task.joined++;
    }

    CHECK(observed == VA_BUSY);
    CHECK(task.joined == 1 && !task.active);
    if (cancel || lost)
    {
        CHECK(status == CVM_HOST_ERROR);
        CHECK(vs_step(&session, &out) == CVM_HOST_ERROR && out.integer == 91);
    }
    else
    {
        CHECK(status == VS_RUNNING);
        CHECK(vs_step(&session, &out) == VS_DONE && out.integer == 15);
    }
    CHECK(va_poll(&task) == VA_STATE && va_cancel(&task) == VA_STATE);
    CHECK(va_destroy(&task) == VA_READY);
    f.gate = 0;
    f.lose_reply = 0;
    CHECK(store_open(&database, io(&f)) == STORE_OK &&
          store_check(&database) == STORE_OK);
    value = 91;
    CHECK(vh_host(&h, 2, 0, &value) == STORE_OK && value == 15);
    unsigned events = f.flushes;
    CHECK(vh_increment(&h, 1, 5, &value) == STORE_OK && value == 15 &&
          f.flushes == events);
    CHECK(!pthread_cond_destroy(&f.condition) &&
          !pthread_mutex_destroy(&f.mutex));
    CHECK(!fclose(stream));
    printf("VA_SCENARIO cancel=%u lost=%u result15 joined1 retry_no_write\n",
           cancel, lost);
    return 0;
}

int main(void)
{
    if (scenario(0, 0) || scenario(1, 0) || scenario(0, 1) || scenario(1, 1))
        return 1;
    puts("VA_PASS real_file_commit_pending_cancel_quiescence");
    return 0;
}
