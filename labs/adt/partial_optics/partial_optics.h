#ifndef ADT_PARTIAL_OPTICS_H
#define ADT_PARTIAL_OPTICS_H
#include "adt.h"
enum adl_status {ADL_OK,ADL_INVALID,ADL_ABSENT,ADL_CALLBACK,ADL_RANGE,ADL_VERSION};
struct adl_inner {unsigned present;struct ad_pair endpoint;uint32_t audit;};
struct adl_config {unsigned present;struct adl_inner inner;uint32_t identity,footer;};
/* All fields initialized, both presence flags0/1. Output is disjoint. */
enum adl_status adl_get(struct adl_config,uint32_t *);
enum adl_status adl_put(struct adl_config,uint32_t,struct adl_config *);
typedef int (*adl_change_fn)(void *,uint32_t,uint32_t *);
struct adl_step {adl_change_fn change;void *context;};
/* Zero-success callbacks run once each left-to-right after full admission.
 * On refusal output is unchanged, but external callback effects remain.
 * No callback may retain the borrowed result or reenter/mutate control/output. */
enum adl_status adl_update(struct adl_config,const struct adl_step *,size_t,struct adl_config *);
enum {ADL_SUCCESS,ADL_IO,ADL_TIMEOUT};
struct adl_error {unsigned version,tag;union {struct ad_pair io;uint32_t timeout;} data;};
struct adl_io {unsigned version;uint32_t code,context;};
/* V1: SUCCESS or IO(code1..255,context0..255).
 * V2: SUCCESS, IO(code1..65535,contextuint32), or TIMEOUT(uint32).
 * Preview matches only IO; review retains version and both payload fields. */
enum adl_status adl_preview(struct adl_error,struct adl_io *);
enum adl_status adl_review(struct adl_io,struct adl_error *);
#endif
