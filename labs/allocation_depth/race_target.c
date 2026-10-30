/* CM-SPECIMEN: defective by design; classify the defect before running
   (see CONTRACTS.md). */
/* Original isolated T16 negative target. No outside process, file or secret. */
#include <pthread.h>
#include <stdio.h>

enum
{
    INCREMENTS = 2000
};

static pthread_mutex_t metadata_lock = PTHREAD_MUTEX_INITIALIZER;
static unsigned metadata;
static unsigned payload;

static void *worker(void *unused)
{
    (void)unused;
    for (unsigned i = 0; i < INCREMENTS; ++i)
    {
        pthread_mutex_lock(&metadata_lock);
        ++metadata;
        pthread_mutex_unlock(&metadata_lock);
        ++payload;
    }
    return 0;
}

int main(void)
{
    pthread_t a;
    pthread_t b;
    if (pthread_create(&a, 0, worker, 0) || pthread_create(&b, 0, worker, 0))
    {
        return 2;
    }
    pthread_join(a, 0);
    pthread_join(b, 0);
    printf("synthetic metadata=%u payload=%u\n", metadata, payload);
    return 0;
}
