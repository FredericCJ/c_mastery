#ifndef LIBRARY_PICOLIB_H
#define LIBRARY_PICOLIB_H
#include <stddef.h>
/* Teaching console stores at most4 output bytes and offers synthetic abc input.
 */
int lp_reset(void);
size_t lp_output(unsigned char out[4]);
#endif
