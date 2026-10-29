#include "cmx_port.h"

void cmx_board_init(void)
{ /* TODO: declared board startup and shared memory */
}

void cmx_amp_service(void)
{ /* TODO: epoch-qualified ownership and doorbells */
}

void HSEM_Handler(void)
{
    cmx_port_finish(3);
}
