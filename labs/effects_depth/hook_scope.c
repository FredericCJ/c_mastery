#include <stddef.h>
#include <stdio.h>
static unsigned requests,hooks;static unsigned char storage[16];
__attribute__((visibility("hidden"))) void*ef_local_alloc(size_t n){++requests;return n<=sizeof storage?storage:NULL;}
void*__wrap_ef_local_alloc(size_t n){++hooks;return n<=sizeof storage?storage:NULL;}
int main(void){unsigned char*p=ef_local_alloc(8);if(!p)return 1;*p=7;if(requests!=1||hooks!=0)return 2;printf("{\"same_translation_unit_requests\":%u,\"link_wrap_hooks\":%u}\n",requests,hooks);return 0;}
