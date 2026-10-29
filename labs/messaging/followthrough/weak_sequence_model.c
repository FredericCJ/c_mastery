#include "weak.h"
#include <pthread.h>
#include <assert.h>
static struct mfw_sequence sequence;

static void *writer(void *unused)
{
    (void)unused;
    assert(mfw_write(&sequence, 1, 2) == MFW_OK);
    assert(mfw_write(&sequence, 3, 6) == MFW_OK);
    return NULL;
}

static void *reader(void *unused)
{
    (void)unused;
    unsigned values[2] = {99, 99};
    enum mfw_status status = mfw_read(&sequence, 1, values);
    if (status == MFW_OK)
    {
        assert(values[1] == 2 * values[0]);
    }
    else
    {
        assert(status == MFW_BUSY && values[0] == 99 && values[1] == 99);
    }
    return NULL;
}

int main(void)
{
    pthread_t a, b;
    mfw_sequence_init(&sequence);
    assert(!pthread_create(&a, NULL, writer, NULL));
    assert(!pthread_create(&b, NULL, reader, NULL));
    assert(!pthread_join(a, NULL));
    assert(!pthread_join(b, NULL));
    return 0;
}
