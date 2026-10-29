#ifndef CMASTERY_OWNERSHIP_COW_RESOURCES_H
#define CMASTERY_OWNERSHIP_COW_RESOURCES_H
#include "ownership.h"
#include <pthread.h>
struct ocr_resource {int32_t *data;size_t length;};
/* Clone initializes an empty output; failure may leave a valid partial owner.
 * Drop is nonfailing, accepts empty, empties the owner. No callback reentry. */
struct ocr_ops {void *context;enum ow_status (*clone)(void *,const struct ocr_resource *,struct ocr_resource *);void (*drop)(void *,struct ocr_resource *);};
struct ocr_memory {void *context;void *(*take)(void *,size_t);void (*release)(void *,void *);};
struct ocr_block;
struct ocr_loan {struct ocr_block *block;unsigned serial,active,retired;};
struct ocr_store {pthread_mutex_t mutex;struct ocr_memory memory;struct ocr_ops ops;struct ocr_block *current;struct ocr_loan loan[4];unsigned ready,closed;};
struct ocr_snapshot {struct ocr_store *owner;unsigned slot,serial;};
/* Fresh nonowning, noncopied store only. All contexts live through destroy. */
enum ow_status ocr_init(struct ocr_store *,struct ocr_memory,struct ocr_ops,const struct ocr_resource seed[3]);
enum ow_status ocr_snapshot(struct ocr_store *,struct ocr_snapshot *);
enum ow_status ocr_read(struct ocr_snapshot,size_t resource,size_t element,int32_t *);
enum ow_status ocr_release(struct ocr_snapshot);
enum ow_status ocr_write(struct ocr_store *,size_t resource,size_t element,int32_t);
enum ow_status ocr_close(struct ocr_store *);
/* Requires externally joined/quiescent callers; live snapshots return BUSY. */
enum ow_status ocr_destroy(struct ocr_store *);
#endif
