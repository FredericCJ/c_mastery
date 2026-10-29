#include "compare.h"
#include "stm32f4xx_hal.h"
static volatile uint32_t clock_value;

uint32_t HAL_GetTick(void)
{
    return clock_value++;
}

static uint32_t tick(void *ctx)
{
    (void)ctx;
    return HAL_GetTick();
}

/* Equal selected fresh-context, preconfigured 8N1 one-byte contracts.
 * This synthetic clock is for linked-cost inspection, not board timing. */
int pl_cost_direct(volatile uint32_t *regs, uint8_t b, uint32_t timeout)
{
    return pl_byte(pl_direct_f439(regs, tick, 0), b, timeout);
}

int pl_cost_generated(volatile uint32_t *regs, uint8_t b, uint32_t timeout)
{
    return pl_byte(pl_generated(regs, 0, tick, 0), b, timeout);
}

int pl_cost_vendor(volatile uint32_t *regs, uint8_t b, uint32_t timeout)
{
    if (!regs || timeout >= 0x80000000u)
        return PL_INVALID;
    UART_HandleTypeDef u = {0};
    u.Instance = (USART_TypeDef *)regs;
    u.Init.WordLength = UART_WORDLENGTH_8B;
    u.Init.Parity = UART_PARITY_NONE;
    u.gState = HAL_UART_STATE_READY;
    HAL_StatusTypeDef r = HAL_UART_Transmit(&u, &b, 1, timeout);
    return r == HAL_OK ? PL_OK : r == HAL_TIMEOUT ? PL_TIMEOUT : PL_IO;
}
