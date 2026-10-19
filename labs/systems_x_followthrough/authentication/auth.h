#ifndef SYSTEMS_X_AUTH_H
#define SYSTEMS_X_AUTH_H
#include "b05.h"
typedef int (*SxMac)(void *key_context,const unsigned char *message,size_t length,unsigned char tag[32]);
typedef struct {B5Redo record;unsigned char tag[32];} SxAuthenticated;
int sx_auth_encode(uint64_t store_id,const B5Redo *,unsigned char message[32]);
int sx_auth_recover(uint64_t store_id,uint32_t checkpoint,uint32_t base,const SxAuthenticated *,size_t count,SxMac,void *key_context,uint32_t *out);
#endif
