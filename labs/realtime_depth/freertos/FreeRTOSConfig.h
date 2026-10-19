#ifndef FC_CONFIG_H
#define FC_CONFIG_H
#include "comparison.h"
#define configUSE_PREEMPTION 1
#define configUSE_TIME_SLICING 1
#define configUSE_IDLE_HOOK 0
#define configUSE_TICK_HOOK 0
#define configTICK_RATE_HZ FC_TICK_RATE
#define configMAX_PRIORITIES 5
#ifdef FC_M4
#define configMINIMAL_STACK_SIZE 512
#define configCPU_CLOCK_HZ 25000000u
#define configKERNEL_INTERRUPT_PRIORITY 255
#define configMAX_SYSCALL_INTERRUPT_PRIORITY 128
#define configCHECK_HANDLER_INSTALLATION 1
#else
#define configMINIMAL_STACK_SIZE 4096
#endif
#define configMAX_TASK_NAME_LEN 16
#define configTICK_TYPE_WIDTH_IN_BITS TICK_TYPE_WIDTH_32_BITS
#define configIDLE_SHOULD_YIELD 1
#define configUSE_MUTEXES 1
#define configUSE_COUNTING_SEMAPHORES 1
#define configUSE_TIMERS 0
#define configSUPPORT_STATIC_ALLOCATION 1
#define configSUPPORT_DYNAMIC_ALLOCATION 0
#define configKERNEL_PROVIDED_STATIC_MEMORY 1
#define configUSE_MINI_LIST_ITEM 0
#define INCLUDE_vTaskDelete 1
#define INCLUDE_vTaskSuspend 1
#define INCLUDE_vTaskDelay 1
#define INCLUDE_uxTaskPriorityGet 1
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#define INCLUDE_xTaskGetSchedulerState 1
#define configASSERT(x) FC_REQUIRE(x)
#if FC_TICK_RATE != 100 && FC_TICK_RATE != 1000
#error unsupported comparison tick rate
#endif
#endif
