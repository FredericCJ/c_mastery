#include "list.h"
#include "once.h"
#include "transfers.h"
#include <stdio.h>
#include <stddef.h>

int main(void)
{
    printf(
        "COST SxList=%zu align=%zu node=%zu reader=%zu epoch=%zu head_offset=%zu node_offset=%zu reader_offset=%zu\n",
        sizeof(struct sx_list), _Alignof(struct sx_list),
        sizeof(struct sx_node), sizeof(struct sx_reader),
        sizeof(struct mb_epoch), offsetof(struct sx_list, head),
        offsetof(struct sx_list, node), offsetof(struct sx_list, reader));
    printf(
        "COST SxOnce=%zu journal=%zu MBOnce=%zu map=%zu ledger=%zu hotplug=%zu\n",
        sizeof(struct sx_once), sizeof(struct sx_journal),
        sizeof(struct mb_once), sizeof(struct sx_map), sizeof(struct sx_ledger),
        sizeof(struct sx_hotplug));
    return 0;
}
