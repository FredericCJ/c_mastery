#ifndef CMASTERY_OWNERSHIP_COW_RESOURCES_H
#define CMASTERY_OWNERSHIP_COW_RESOURCES_H
#include "ownership.h"
#include <pthread.h>
typedef struct {int32_t *data;size_t length;} OcrResource;
/* Clone initializes an empty output; failure may leave a valid partial owner.
 * Drop is nonfailing, accepts empty, empties the owner. No callback reentry. */
typedef struct {void *context;OwStatus (*clone)(void *,const OcrResource *,OcrResource *);void (*drop)(void *,OcrResource *);} OcrOps;
typedef struct {void *context;void *(*take)(void *,size_t);void (*release)(void *,void *);} OcrMemory;
typedef struct OcrBlock OcrBlock;
typedef struct {OcrBlock *block;unsigned serial,active,retired;} OcrLoan;
typedef struct OcrStore {pthread_mutex_t mutex;OcrMemory memory;OcrOps ops;OcrBlock *current;OcrLoan loan[4];unsigned ready,closed;} OcrStore;
typedef struct {OcrStore *owner;unsigned slot,serial;} OcrSnapshot;
/* Fresh nonowning, noncopied store only. All contexts live through destroy. */
OwStatus ocr_init(OcrStore *,OcrMemory,OcrOps,const OcrResource seed[3]);
OwStatus ocr_snapshot(OcrStore *,OcrSnapshot *);
OwStatus ocr_read(OcrSnapshot,size_t resource,size_t element,int32_t *);
OwStatus ocr_release(OcrSnapshot);
OwStatus ocr_write(OcrStore *,size_t resource,size_t element,int32_t);
OwStatus ocr_close(OcrStore *);
/* Requires externally joined/quiescent callers; live snapshots return BUSY. */
OwStatus ocr_destroy(OcrStore *);
#endif
