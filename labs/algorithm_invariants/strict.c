#include "invariants.h"
int ai_strict(void){AIState state={1071,462},next;const AIRecord a[2]={{2,1},{1,2}},b[2]={{1,2},{2,1}};unsigned yes=0;return ai_step(state,0,&next)!=AI_OK || next.a!=462 || next.b!=147 || ai_permutation(a,b,2,&yes)!=AI_OK || !yes;}
