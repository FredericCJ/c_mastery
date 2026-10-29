#ifndef CMASTERY_CHANNEL_FOLLOW_H
#define CMASTERY_CHANNEL_FOLLOW_H
#include "async_follow.h"
#include "messaging.h"
int ca_channel_deliver(struct ca_ticket *, struct msg_hub *, unsigned wait);
#endif
