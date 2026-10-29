#include "stm32f4xx_hal.h"
void pl_report(const char *);
#define CHECK(x)                                \
    do                                          \
    {                                           \
        if (!(x))                               \
        {                                       \
            pl_report("PL_CALLBACK_CHECK:" #x); \
            return 1;                           \
        }                                       \
    } while (0)
static unsigned callbacks, ready_seen, macro_calls;

static uint32_t flag_expression(void)
{
    macro_calls++;
    return UART_FLAG_TXE;
}

static uint32_t interrupt_expression(void)
{
    macro_calls++;
    return UART_IT_TXE;
}

static HAL_StatusTypeDef nested;
static const uint8_t next_byte = 9;

uint32_t HAL_GetTick(void)
{
    return 0;
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *u)
{
    callbacks++;
    ready_seen += u->gState == HAL_UART_STATE_READY;
    if (callbacks == 1)
    {
        nested = HAL_UART_Transmit_IT(u, &next_byte, 1);
    }
}

int main(void)
{
    USART_TypeDef d = {0};
    UART_HandleTypeDef u = {0};
    u.Instance = &d;
    u.gState = HAL_UART_STATE_READY;
    u.Init.WordLength = UART_WORDLENGTH_8B;
    u.Init.Parity = UART_PARITY_NONE;
    d.SR = USART_SR_TXE;
    macro_calls = 0;
    CHECK(__HAL_UART_GET_FLAG(&u, flag_expression()) && macro_calls == 2);
    macro_calls = 0;
    __HAL_UART_ENABLE_IT(&u, interrupt_expression());
    CHECK(macro_calls == 2 && (d.CR1 & USART_CR1_TXEIE));
    d.CR1 = 0;
    d.SR = 0;
    static const uint8_t first = 7;
    CHECK(HAL_UART_Transmit_IT(&u, &first, 1) == HAL_OK);
    CHECK(u.gState == HAL_UART_STATE_BUSY_TX && (d.CR1 & USART_CR1_TXEIE));
    d.SR = USART_SR_TXE;
    HAL_UART_IRQHandler(&u);
    CHECK(d.DR == 7 && !u.TxXferCount && !(d.CR1 & USART_CR1_TXEIE) &&
          (d.CR1 & USART_CR1_TCIE));
    CHECK(!callbacks); /* DR publication is not completion. */
    d.SR = USART_SR_TC;
    HAL_UART_IRQHandler(&u);
    CHECK(callbacks == 1 && ready_seen == 1 && nested == HAL_OK &&
          u.gState == HAL_UART_STATE_BUSY_TX && u.pTxBuffPtr == &next_byte);
    /* No trailing cleanup in the old operation may reset the newly started one.
     */
    d.SR = USART_SR_TXE;
    HAL_UART_IRQHandler(&u);
    CHECK(d.DR == 9 && !u.TxXferCount);
    d.SR = USART_SR_TC;
    HAL_UART_IRQHandler(&u);
    CHECK(callbacks == 2 && ready_seen == 2 &&
          u.gState == HAL_UART_STATE_READY);
    pl_report("PL_CALLBACK_PASS");
    return 0;
}
