#include "comparison.h"
#include <limits.h>
int fc_ticks(unsigned ms,unsigned frequency,unsigned *out){
 if(!out||!frequency||frequency>10000u||ms>UINT_MAX/frequency)return 0;
 unsigned product=ms*frequency,value=product/1000u;
 if(FC_MUTANT!=2&&product%1000u)++value;
 if(value==UINT_MAX)return 0;
 *out=value;return 1;
}
unsigned fc_priority(unsigned urgency){if(urgency<1u||urgency>3u)return 0;return FC_MUTANT==1?urgency:4u-urgency;}
