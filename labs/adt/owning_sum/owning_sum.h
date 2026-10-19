#ifndef CMASTERY_AD_OWNING_SUM_H
#define CMASTERY_AD_OWNING_SUM_H
#include "adt.h"
typedef enum { ADO_OK, ADO_INVALID, ADO_NOMEM, ADO_UNIMPLEMENTED } AdoStatus;
typedef enum { ADO_EMPTY, ADO_NUMBER, ADO_PAIR, ADO_TEXT } AdoTag;
typedef int (*AdoClone)(void *, const unsigned char *, size_t, unsigned char **);
typedef void (*AdoDrop)(void *, unsigned char *, size_t);
typedef struct { AdoClone clone; AdoDrop drop; void *context; } AdoOps;
typedef struct { unsigned char *bytes; size_t length; AdoOps ops; } AdoText;
typedef struct {
    AdoTag tag;
    union { uint32_t number; ADPair pair; AdoText text; } data;
} AdoValue;
/* Init only unowned storage. All other controls are live, admitted owners. */
void ado_init(AdoValue *);
AdoStatus ado_from_value(AdoValue *, ADValue);
AdoStatus ado_text(AdoValue *, const unsigned char *, size_t, AdoOps);
AdoStatus ado_copy(AdoValue *, const AdoValue *);
AdoStatus ado_move(AdoValue *, AdoValue *);
void ado_drop(AdoValue *);
AdoStatus ado_measure(const AdoValue *, uint64_t *);
AdoStatus ado_number(const AdoValue *, uint32_t *);
#endif
