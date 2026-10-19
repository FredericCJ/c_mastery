#ifndef CMASTERY_DISPATCH_BROKER_H
#define CMASTERY_DISPATCH_BROKER_H
#include "dispatch.h"
#include <stdint.h>
#include <limits.h>
_Static_assert(CHAR_BIT==8,"broker wire requires eight-bit octets");
#define DS_BROKER_FRAME 16u
enum DSBrokerOperation { DS_B_READ=1,DS_B_ADD=2,DS_B_DERIVE=3,DS_B_DEPUTY=4 };
typedef struct { unsigned holder,target,rights,epoch,live; } DSBrokerGrant;
typedef struct {
  uint32_t counters[2];unsigned epoch[2],next_sequence[2];DSBrokerGrant grants[4];
} DSBroker;
void ds_broker_init(DSBroker *);
int ds_broker_revoke(DSBroker *,unsigned);
int ds_broker_apply(DSBroker *,unsigned,const unsigned char [16],unsigned char [16]);
void ds_broker_request(unsigned char [16],unsigned,unsigned,unsigned,uint32_t,unsigned,unsigned,uint32_t);
uint32_t ds_broker_u32(const unsigned char [4]);
#endif
