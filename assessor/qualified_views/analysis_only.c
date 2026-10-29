#include "views.h"
const int *escape(void){return (int[]){1,2,3,4};}
struct fv_view wrapped_escape(void){return (struct fv_view){(int[]){1,2,3,4},4};}
int indeterminate(void){struct fv_record uninitialized;return uninitialized.values[0];}
/* Never call these functions. Structure value return copies the pointer, not its pointee. */
