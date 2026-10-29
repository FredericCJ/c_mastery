#include "bound.h"
uint32_t ab_callback_entry(const uint32_t *a,uint32_t n,uint32_t key){struct ab_context context={0,0,0x13579bdu};uint32_t result=ab_callback_lower(a,n,key,ab_poison,&context);return context.misaligned?UINT32_MAX:result;}
