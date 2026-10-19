#ifndef PL_SELECTED_HAL_CONF
#define PL_SELECTED_HAL_CONF
#define HAL_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define USE_RTOS 0U
#define USE_HAL_UART_REGISTER_CALLBACKS 0U
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_uart.h"
/* Preconfigured selected 8N1 bounded comparison, not HAL initialization. */
#define assert_param(x) ((void)0U)
#endif
