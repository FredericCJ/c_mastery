#include "comparison.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <limits.h>
#ifdef FC_M4
#define FC_STACK 1024
#else
#define FC_STACK 4096
#endif
static StaticTask_t controls[4];
static StackType_t stacks[4][FC_STACK];
static TaskHandle_t tasks[4];
static StaticQueue_t queue_control, mutex_control, second_control;
static unsigned char storage[8 * sizeof(unsigned)];
static QueueHandle_t queue;
static SemaphoreHandle_t mutex, second;
static struct fc_report *report;
static volatile unsigned command, isr_after;

static void note(unsigned value)
{
    taskENTER_CRITICAL();
    FC_REQUIRE(report->traces < 3);
    report->trace[report->traces++] = value;
    taskEXIT_CRITICAL();
}
#ifdef FC_M4
void fc_irq(void)
{
    BaseType_t wake = pdFALSE;
    unsigned value = 777;
    FC_REQUIRE(xQueueSendFromISR(queue, &value, &wake) == pdPASS);
    FC_REQUIRE(wake == pdTRUE);
    report->isr = 1;
    if (FC_MUTANT != 4)
        portYIELD_FROM_ISR(wake);
}

static void irq_raise(void)
{
    *(volatile unsigned char *)0xe000e400u = 160;
    *(volatile unsigned *)0xe000e100u = 1;
    *(volatile unsigned *)0xe000e200u = 1;
    __asm__ volatile("dsb;isb" ::: "memory");
}
#endif
static void low(void *unused)
{
    (void)unused;
    for (;;)
    {
        FC_REQUIRE(ulTaskNotifyTake(pdTRUE, portMAX_DELAY));
        if (command == 1)
        {
            for (unsigned n = 0; n < 256; ++n)
                FC_REQUIRE(xQueueSend(queue, &n, portMAX_DELAY) == pdPASS);
            unsigned end = UINT_MAX;
            FC_REQUIRE(xQueueSend(queue, &end, portMAX_DELAY) == pdPASS);
            xTaskNotifyGive(tasks[0]);
        }
        else if (command == 2)
        {
            FC_REQUIRE(xSemaphoreTake(mutex, 0) == pdPASS);
            FC_REQUIRE(xSemaphoreTake(second, 0) == pdPASS);
            xTaskNotifyGive(tasks[0]);
            FC_REQUIRE(ulTaskNotifyTake(pdTRUE, portMAX_DELAY));
            report->boosted = uxTaskPriorityGet(NULL) == fc_priority(1);
            note(1);
            FC_REQUIRE(xSemaphoreGive(mutex) == pdPASS);
            report->nested_retained = uxTaskPriorityGet(NULL) == fc_priority(1);
            FC_REQUIRE(xSemaphoreGive(second) == pdPASS);
            report->restored = uxTaskPriorityGet(NULL) == fc_priority(3);
            xTaskNotifyGive(tasks[0]);
        }
        else if (command == 3)
        {
#ifdef FC_M4
            irq_raise();
            isr_after = 1;
#else
            unsigned value = 777;
            FC_REQUIRE(xQueueSend(queue, &value, 0) == pdPASS);
#endif
        }
        else
            fc_fail(__LINE__);
    }
}

static void high(void *unused)
{
    (void)unused;
    for (;;)
    {
        FC_REQUIRE(ulTaskNotifyTake(pdTRUE, portMAX_DELAY));
        FC_REQUIRE(xSemaphoreTake(mutex, portMAX_DELAY) == pdPASS);
        note(2);
        FC_REQUIRE(xSemaphoreGive(mutex) == pdPASS);
        xTaskNotifyGive(tasks[0]);
    }
}

static void medium(void *unused)
{
    (void)unused;
    for (;;)
    {
        FC_REQUIRE(ulTaskNotifyTake(pdTRUE, portMAX_DELAY));
        note(3);
        xTaskNotifyGive(tasks[0]);
    }
}

static void controller(void *unused)
{
    (void)unused;
    unsigned ticks, value;
    FC_REQUIRE(fc_ticks(21, configTICK_RATE_HZ, &ticks));
    report->ticks = ticks;
    FC_REQUIRE(pdMS_TO_TICKS(21) == (21u * configTICK_RATE_HZ) / 1000u);
    for (unsigned n = 0; n < 8; ++n)
        FC_REQUIRE(xQueueSend(queue, &n, 0) == pdPASS);
    TickType_t before = xTaskGetTickCount();
    value = 999;
    report->full = xQueueSend(queue, &value, ticks) == errQUEUE_FULL;
    FC_REQUIRE((TickType_t)(xTaskGetTickCount() - before) >= ticks);
    for (unsigned n = 0; n < 8; ++n)
    {
        FC_REQUIRE(xQueueReceive(queue, &value, 0) == pdPASS);
        FC_REQUIRE(value == n);
    }
    value = 999;
    before = xTaskGetTickCount();
    report->empty = xQueueReceive(queue, &value, ticks) == errQUEUE_EMPTY;
    report->elapsed = (unsigned)(TickType_t)(xTaskGetTickCount() - before);
    FC_REQUIRE(value == 999);
    command = 1;
    xTaskNotifyGive(tasks[1]);
    for (;;)
    {
        FC_REQUIRE(xQueueReceive(queue, &value, portMAX_DELAY) == pdPASS);
        if (value == UINT_MAX)
            break;
        FC_REQUIRE(value == report->count);
        ++report->count;
        report->sum += value;
    }
    FC_REQUIRE(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
    FC_REQUIRE(uxQueueMessagesWaiting(queue) == 0);
    command = 2;
    xTaskNotifyGive(tasks[1]);
    FC_REQUIRE(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
    vTaskSuspendAll();
    xTaskNotifyGive(tasks[2]);
    xTaskNotifyGive(tasks[3]);
    xTaskNotifyGive(tasks[1]);
    (void)xTaskResumeAll();
    unsigned finished = 0;
    while (finished < 3)
        finished += (unsigned)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    FC_REQUIRE(finished == 3);
    command = 3;
    xTaskNotifyGive(tasks[1]);
    FC_REQUIRE(xQueueReceive(queue, &value, portMAX_DELAY) == pdPASS &&
               value == 777);
#ifdef FC_M4
    FC_REQUIRE(report->isr == 1 && isr_after == 0);
#else
    (void)isr_after;
#endif
    for (unsigned n = 1; n < 4; ++n)
        vTaskDelete(tasks[n]);
    FC_REQUIRE(uxQueueMessagesWaiting(queue) == 0);
    vQueueDelete(queue);
    vSemaphoreDelete(mutex);
    vSemaphoreDelete(second);
    report->closed = 1;
#ifdef FC_M4
    fc_done(report);
    for (;;)
    {
    }
#else
    vTaskEndScheduler();
    for (;;)
    {
    }
#endif
}

void fc_run(struct fc_report *r)
{
    report = r;
    queue = xQueueCreateStatic(8, sizeof(unsigned), storage, &queue_control);
    mutex = FC_MUTANT == 3 ? xSemaphoreCreateBinaryStatic(&mutex_control)
                           : xSemaphoreCreateMutexStatic(&mutex_control);
    second = xSemaphoreCreateMutexStatic(&second_control);
    FC_REQUIRE(queue && mutex && second);
    if (FC_MUTANT == 3)
        FC_REQUIRE(xSemaphoreGive(mutex) == pdPASS);
    tasks[0] = xTaskCreateStatic(controller, "control", FC_STACK, 0, 4,
                                 stacks[0], &controls[0]);
    tasks[1] = xTaskCreateStatic(low, "low", FC_STACK, 0, fc_priority(3),
                                 stacks[1], &controls[1]);
    tasks[2] = xTaskCreateStatic(high, "high", FC_STACK, 0, fc_priority(1),
                                 stacks[2], &controls[2]);
    tasks[3] = xTaskCreateStatic(medium, "middle", FC_STACK, 0, fc_priority(2),
                                 stacks[3], &controls[3]);
    for (unsigned n = 0; n < 4; ++n)
    {
        FC_REQUIRE(tasks[n]);
    }
    vTaskStartScheduler();
}
