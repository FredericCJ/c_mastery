#ifndef LP_REPLACEMENT_H
#define LP_REPLACEMENT_H
#include <stddef.h>
struct le_ops {void *context;void *(*acquire)(void *,unsigned,size_t);void (*release)(void *,unsigned,void *,size_t);};
struct le_owner {unsigned char *data;size_t size;unsigned id;struct le_ops ops;};
enum le_status {LE_OK,LE_INVALID,LE_NOMEM};
/* out is initially empty; ops/context live through final transferred-owner drop.
   Callbacks do not reenter/mutate control and return normally; release cannot fail. */
enum le_status le_run(struct le_ops ops,unsigned transfer,struct le_owner *out);
/* Empty before callback; repeated drop is a no-op. */
void le_drop(struct le_owner *owner);
#endif
