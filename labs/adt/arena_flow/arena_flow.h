#ifndef CMASTERY_ADF_H
#define CMASTERY_ADF_H
#include "adt.h"
typedef enum {ADF_OK,ADF_INVALID,ADF_BUDGET,ADF_NO_FIXED_POINT} AdfStatus;
typedef struct {ADNode nodes[32];uint32_t values[32];size_t count;} AdfArena;
/* Copy then validate the entire arena, including unreachable nodes. Certificate
   owns inline storage; clients must not mutate its public representation. */
AdfStatus adf_arena(const ADNode *,size_t,AdfArena *);
AdfStatus adf_root(const AdfArena *,ADNodeIndex,uint32_t *);
typedef struct {unsigned predecessors,generate,keep;} AdfNode;
typedef struct {AdfNode nodes[8];unsigned count,values[8],pending,evaluations;} AdfSession;
typedef struct {unsigned values[8],evaluations;} AdfResult;
/* L=(powerset of8facts)^count, bottom0; each transfer is
   generate | ((union of predecessors) & keep). Count1..8, all masks checked. */
AdfStatus adf_start(const AdfNode *,unsigned,AdfSession *);
/* Progress commits one popped node at a time. BUDGET preserves output, retains
   resumable session progress. OK publishes a fixed point; repeated OK is stable. */
AdfStatus adf_step(AdfSession *,unsigned budget,AdfResult *);
typedef struct {unsigned left,right,table;} AdfBoolean;
/* Separate arbitrary Boolean-equation model, count1..4. Enumerate assignments
   in numeric order. Only a complete search proves NO_FIXED_POINT. This is not
   the finite monotone solver's state space or a claim of static analysis. */
AdfStatus adf_boolean(const AdfBoolean *,unsigned count,unsigned budget,unsigned *assignment);
#endif
