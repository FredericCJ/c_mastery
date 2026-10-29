#include "service.h"
#include <stdio.h>
#include <string.h>
static unsigned char live[STORE_IMAGE_PAGES][STORE_PAGE_SIZE],
    durable[STORE_IMAGE_PAGES][STORE_PAGE_SIZE];
static store database;
static unsigned writes, flushes;

static int read_page(void *p, uint32_t n, void *out)
{
    (void)p;
    if (n >= STORE_IMAGE_PAGES)
        return -1;
    memcpy(out, live[n], STORE_PAGE_SIZE);
    return 0;
}

static int write_page(void *p, uint32_t n, const void *in)
{
    (void)p;
    if (n >= STORE_IMAGE_PAGES)
        return -1;
    memcpy(live[n], in, STORE_PAGE_SIZE);
    writes++;
    return 0;
}

static int flush(void *p)
{
    (void)p;
    memcpy(durable, live, sizeof live);
    flushes++;
    return 0;
}

#define C(x)                                              \
    do                                                    \
    {                                                     \
        if (!(x))                                         \
        {                                                 \
            printf("SFS_CHECK:%u\n", (unsigned)__LINE__); \
            return 1;                                     \
        }                                                 \
    } while (0)

static struct sfs_request request(unsigned op, uint64_t id, unsigned client)
{
    struct sfs_request q = {0};
    q.operation = op;
    q.session = 1;
    q.id = id;
    q.client = client;
    q.epoch = 1;
    return q;
}

int main(void)
{
    store_io io = {0, read_page, write_page, flush, STORE_IMAGE_PAGES};
    C(!store_format(io) && !store_open(&database, io) &&
      !sdr_initialize(&database));
    struct sfs_reply r = {0}, sentinel;
    memset(&sentinel, 0xa5, sizeof sentinel);
    r = sentinel;
    struct sfs_request q = request(SFS_PUT, 1, 1);
    q.klen = 17;
    C(sfs_apply(&database, &q, &r) == SFS_ARGUMENT &&
      !memcmp(&r, &sentinel, sizeof r));
    q.klen = 4;
    q.vlen = 4;
    memcpy(q.key, "left", 4);
    q.value[0] = 7;
    C(!sfs_apply(&database, &q, &r) && !r.status);
    q = request(SFS_PUT, 1, 2);
    q.klen = 5;
    q.vlen = 4;
    memcpy(q.key, "right", 5);
    q.value[0] = 9;
    C(!sfs_apply(&database, &q, &r) && !r.status);
    q = request(SFS_BATCH, 2, 1);
    C(!sfs_apply(&database, &q, &r) && !r.status && r.length == 8 &&
      r.value[0] == 7 && r.value[4] == 9);
    int32_t expected = 0;
    for (unsigned i = 1; i <= 200; i++)
    {
        unsigned client = 1 + (i & 1);
        int32_t delta = (int32_t)(i % 7) - 3;
        q = request(SFS_INCREMENT, i, client);
        q.delta = delta;
        expected += delta;
        unsigned prior_flush = flushes;
        C(!sfs_apply(&database, &q, &r) && !r.status && r.result == expected &&
          flushes > prior_flush);
        unsigned prior_writes = writes;
        C(!sfs_apply(&database, &q, &r) && !r.status && r.result == expected &&
          writes == prior_writes);
        q.delta++;
        C(!sfs_apply(&database, &q, &r) && r.status == SDR_CONFLICT &&
          writes == prior_writes);
        q.delta--;
        if (i % 17 == 0)
        {
            memcpy(live, durable, sizeof live);
            C(!store_open(&database, io));
            C(!sfs_apply(&database, &q, &r) && !r.status &&
              r.result == expected);
        }
    }
    q = request(SFS_INCREMENT, 1, 2);
    q.delta = -2;
    C(!sfs_apply(&database, &q, &r) && r.status == SDR_STALE);
    q = request(SFS_INCREMENT, 201, 1);
    q.epoch = 2;
    C(!sfs_apply(&database, &q, &r) && r.status == SDR_STALE);
    q = request(SFS_VALUE, 202, 1);
    C(!sfs_apply(&database, &q, &r) && !r.status && r.result == expected &&
      store_check(&database) == STORE_OK);
    printf(
        "PASS sfs:200 durable effects 200 exact retries 200 payload conflicts 11 reopenings writes=%u flushes=%u\n",
        writes, flushes);
    return 0;
}
