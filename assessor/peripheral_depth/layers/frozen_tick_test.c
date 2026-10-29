#include "stm32f4xx_hal.h"
void pl_report(const char *);
void pl_stop(unsigned);
static unsigned ticks;

uint32_t HAL_GetTick(void)
{
    if (++ticks == 4096)
    {
        pl_report("PL_FROZEN_4096_WAIT_ITERATIONS");
        pl_stop(42);
    }
    return 0;
}

int main(void)
{
    USART_TypeDef d = {0};
    UART_HandleTypeDef u = {0};
    u.Instance = &d;
    u.gState = HAL_UART_STATE_READY;
    const uint8_t b = 7;
    (void)HAL_UART_Transmit(&u, &b, 1, 1);
    pl_report("PL_FROZEN_UNEXPECTED_RETURN");
    return 1;
}
