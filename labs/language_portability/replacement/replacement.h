#ifndef LP_REPLACEMENT_H
#define LP_REPLACEMENT_H
#include <stddef.h>
typedef struct {void *context;void *(*acquire)(void *,unsigned,size_t);void (*release)(void *,unsigned,void *,size_t);} LeOps;
typedef struct {unsigned char *data;size_t size;unsigned id;LeOps ops;} LeOwner;
typedef enum {LE_OK,LE_INVALID,LE_NOMEM} LeStatus;
/* out is initially empty; ops/context live through final transferred-owner drop.
   Callbacks do not reenter/mutate control and return normally; release cannot fail. */
LeStatus le_run(LeOps ops,unsigned transfer,LeOwner *out);
/* Empty before callback; repeated drop is a no-op. */
void le_drop(LeOwner *owner);
#endif
