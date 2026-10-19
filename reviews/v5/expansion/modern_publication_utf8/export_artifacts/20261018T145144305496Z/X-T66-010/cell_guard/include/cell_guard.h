#ifndef CMASTERY_CELL_GUARD_H
#define CMASTERY_CELL_GUARD_H
#include "ownership.h"
#include <pthread.h>
typedef struct OcgCell {pthread_mutex_t mutex;OwCell cell;unsigned ready,closed,updating;} OcgCell;
typedef struct {OcgCell *owner;OwToken token;} OcgToken;
typedef int (*OcgUpdate)(void *,int32_t old,int32_t *candidate);
/* Fresh fixed-address storage; init returns actual pthread_mutex_init result. */
int ocg_init(OcgCell *,int32_t);
OwStatus ocg_acquire(OcgCell *,int,OcgToken *);
OwStatus ocg_release(OcgCell *,OcgToken);
OwStatus ocg_read(OcgCell *,OcgToken,int32_t *);
OwStatus ocg_write(OcgCell *,OcgToken,int32_t);
OwStatus ocg_replace(OcgCell *,int32_t,int32_t *old);
/* Callback outside mutex with exclusive guard retained; zero accepts candidate. */
OwStatus ocg_update(OcgCell *,OcgUpdate,void *,int32_t *old);
OwStatus ocg_close(OcgCell *);
/* External join/quiescence required even when no logical guards remain. */
OwStatus ocg_destroy(OcgCell *);
#endif
