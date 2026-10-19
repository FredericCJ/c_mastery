#ifndef CMASTERY_OPTION_TYPES_H
#define CMASTERY_OPTION_TYPES_H
#include <stdint.h>
#include "imports/v4_algo.h"
enum { EOP_OK,EOP_INVALID,EOP_ABSENT,EOP_BUSY,EOP_NOMEM };
enum { EOP_NONE,EOP_SOME };
typedef struct {unsigned tag;uint32_t value;} EopScalar;
typedef struct {unsigned tag;union {struct {uint32_t id;int32_t *cell;V4Allocator allocator;} some;} as;} EopOwner;
typedef enum { EOP_UNIT=0 } EopUnit;
#endif
