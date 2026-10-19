#ifndef PL_SELECTED_G4_HAL_CONF
#define PL_SELECTED_G4_HAL_CONF
#define HAL_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define USE_RTOS 0U
#define USE_HAL_UART_REGISTER_CALLBACKS 0U
/* Required to translate unused vendor configuration functions; no clock setup
 * is executed in the selected preconfigured register-model experiment. */
#define HSI_VALUE 16000000U
#define LSE_VALUE 32768U
#include "stm32g4xx_hal_dma.h"
#include "stm32g4xx_hal_uart.h"
#include "stm32g4xx_hal_rcc.h"
#define assert_param(x) ((void)0U)
#endif
