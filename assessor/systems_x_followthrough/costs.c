#include "list.h"
#include "once.h"
#include "transfers.h"
#include <stdio.h>
#include <stddef.h>
int main(void){printf("COST SxList=%zu align=%zu node=%zu reader=%zu epoch=%zu head_offset=%zu node_offset=%zu reader_offset=%zu\n",sizeof(SxList),_Alignof(SxList),sizeof(SxNode),sizeof(SxReader),sizeof(MBEpoch),offsetof(SxList,head),offsetof(SxList,node),offsetof(SxList,reader));printf("COST SxOnce=%zu journal=%zu MBOnce=%zu map=%zu ledger=%zu hotplug=%zu\n",sizeof(SxOnce),sizeof(SxJournal),sizeof(MBOnce),sizeof(SxMap),sizeof(SxLedger),sizeof(SxHotplug));return 0;}
