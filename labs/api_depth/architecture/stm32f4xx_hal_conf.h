#ifndef AR_SELECTED_HAL_CONF
#define AR_SELECTED_HAL_CONF
#define HAL_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED
#define USE_RTOS 0U
#include "stm32f4xx_hal_gpio.h"
#include "stm32f4xx_hal_rcc.h"
/* Both compared adapters validate the same selected pin/configuration before
 * calling the driver. This narrow build disables vendor assertion callbacks. */
#define assert_param(x) ((void)0U)
#endif
