#include "ring.h"
#include "hashing.h"
int cr_sha_provider(void *context,const unsigned char *bytes,size_t length,unsigned char digest[32])
{(void)context;return rph_hash(bytes,length,digest)==RPH_OK?0:1;}
