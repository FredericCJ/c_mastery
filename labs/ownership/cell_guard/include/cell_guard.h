#ifndef CMASTERY_CELL_GUARD_H
#define CMASTERY_CELL_GUARD_H
#include "ownership.h"
#include <pthread.h>

struct ocg_cell
{
    pthread_mutex_t mutex;
    struct ow_cell cell;
    unsigned ready, closed, updating;
};

struct ocg_token
{
    struct ocg_cell *owner;
    struct ow_token token;
};

typedef int (*ocg_update_fn)(void *, int32_t old, int32_t *candidate);
/* Fresh fixed-address storage; init returns actual pthread_mutex_init result. */
int ocg_init(struct ocg_cell *, int32_t);
enum ow_status ocg_acquire(struct ocg_cell *, int, struct ocg_token *);
enum ow_status ocg_release(struct ocg_cell *, struct ocg_token);
enum ow_status ocg_read(struct ocg_cell *, struct ocg_token, int32_t *);
enum ow_status ocg_write(struct ocg_cell *, struct ocg_token, int32_t);
enum ow_status ocg_replace(struct ocg_cell *, int32_t, int32_t *old);
/* Callback outside mutex with exclusive guard retained; zero accepts candidate. */
enum ow_status ocg_update(struct ocg_cell *, ocg_update_fn, void *,
                          int32_t *old);
enum ow_status ocg_close(struct ocg_cell *);
/* External join/quiescence required even when no logical guards remain. */
enum ow_status ocg_destroy(struct ocg_cell *);
#endif
