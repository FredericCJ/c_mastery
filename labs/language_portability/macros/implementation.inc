#include "macros.h"
#define VALUE 7
#define LM_COMMA (void)1,7
typedef struct {int first,second;} LmPair;
const char *lm_direct(void){return LM_STRING_RAW(VALUE);}
const char *lm_expanded(void){return LM_STRING(VALUE);}
const char *lm_escaped(void){return LM_STRING_RAW( alpha   "q\"x" + beta );}
int lm_paste(int expanded){const int item_VALUE=11,item_7=22;(void)item_VALUE;(void)item_7;return expanded?LM_CAT(item_,VALUE):LM_CAT_RAW(item_,VALUE);}
int lm_forward(void){return LM_FORWARD(LM_COMMA);}
int lm_compound(void){return LM_FIRST(((LmPair){3,4}));}
