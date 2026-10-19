#ifndef CMASTERY_NESTED_VALIDATION_H
#define CMASTERY_NESTED_VALIDATION_H
#include "errors.h"
typedef struct {size_t count;er_config item[3];uint32_t limit;} EvvInput;
typedef struct {unsigned code,depth,path[3];} EvvDiagnostic;
typedef struct {size_t total,stored;unsigned truncated,ratio_checks;} EvvSummary;
/* root!=0; count0..3; cap0..9; ratio is exact rational, not truncated division.
   Live initialized disjoint objects; see VALIDATION.md for ordered phases. */
EvvSummary evv_validate(const EvvInput *,unsigned root,EvvInput *,EvvDiagnostic *,size_t capacity);
#endif
