/* Compile and reason only: this function MUST NOT be called. Alignment does
 * not change the declared unsigned-char object into an AWRecord. */
#include "bytes.h"
int32_t ab_bad_cast(void){_Alignas(AWRecord) unsigned char storage[sizeof(AWRecord)]={0};return ((AWRecord*)(void*)storage)->id;}
