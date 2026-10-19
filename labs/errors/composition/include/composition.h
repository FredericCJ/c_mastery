#ifndef CMASTERY_OWNED_COMPOSITION_H
#define CMASTERY_OWNED_COMPOSITION_H
#include "owning_option.h"
enum { ERC_OK,ERC_INVALID,ERC_BUSY,ERC_NOMEM,ERC_FULL,ERC_NOT_IMPLEMENTED };
enum { ERC_EMPTY,ERC_VALUE,ERC_ERROR };
typedef struct {uint32_t domain,code;size_t offset;} ErcCause;
typedef struct {unsigned count;ErcCause cause[4];} ErcError;
typedef struct {unsigned tag;union {EopOwner owner;ErcError error;} as;} ErcResult;
typedef int32_t (*ErcMap)(int32_t,void *);
/* All controls live, disjoint, serialized, noncopied while owning; see COMPOSITION.md. */
int erc_cause(ErcCause root,ErcError *out);
int erc_wrap(const ErcError *inner,ErcCause outer,ErcError *out);
int erc_error(const ErcError *error,ErcResult *empty);
int erc_adopt(EopOwner *source,ErcResult *empty);
int erc_map_clone(const ErcResult *source,ErcMap fn,void *context,ErcResult *empty);
int erc_drop(ErcResult *result);
#endif
