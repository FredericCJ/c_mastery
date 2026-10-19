#include "views.h"
const int *escape(void){return (int[]){1,2,3,4};}
FvView wrapped_escape(void){return (FvView){(int[]){1,2,3,4},4};}
int indeterminate(void){FvRecord uninitialized;return uninitialized.values[0];}
/* Never call these functions. Structure value return copies the pointer, not its pointee. */
