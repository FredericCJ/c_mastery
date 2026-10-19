#ifndef ADT_PARTIAL_OPTICS_H
#define ADT_PARTIAL_OPTICS_H
#include "adt.h"
typedef enum {ADL_OK,ADL_INVALID,ADL_ABSENT,ADL_CALLBACK,ADL_RANGE,ADL_VERSION} AdlStatus;
typedef struct {unsigned present;ADPair endpoint;uint32_t audit;} AdlInner;
typedef struct {unsigned present;AdlInner inner;uint32_t identity,footer;} AdlConfig;
/* All fields initialized, both presence flags0/1. Output is disjoint. */
AdlStatus adl_get(AdlConfig,uint32_t *);
AdlStatus adl_put(AdlConfig,uint32_t,AdlConfig *);
typedef int (*AdlChange)(void *,uint32_t,uint32_t *);
typedef struct {AdlChange change;void *context;} AdlStep;
/* Zero-success callbacks run once each left-to-right after full admission.
 * On refusal output is unchanged, but external callback effects remain.
 * No callback may retain the borrowed result or reenter/mutate control/output. */
AdlStatus adl_update(AdlConfig,const AdlStep *,size_t,AdlConfig *);
enum {ADL_SUCCESS,ADL_IO,ADL_TIMEOUT};
typedef struct {unsigned version,tag;union {ADPair io;uint32_t timeout;} data;} AdlError;
typedef struct {unsigned version;uint32_t code,context;} AdlIO;
/* V1: SUCCESS or IO(code1..255,context0..255).
 * V2: SUCCESS, IO(code1..65535,contextuint32), or TIMEOUT(uint32).
 * Preview matches only IO; review retains version and both payload fields. */
AdlStatus adl_preview(AdlError,AdlIO *);
AdlStatus adl_review(AdlIO,AdlError *);
#endif
