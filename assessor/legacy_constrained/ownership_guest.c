#include "target.h"
#include "ownership.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <stdatomic.h>

struct lc_message
{
    struct cr_payload *payload;
    struct cr_token token;
};
static struct cr_pool pool;
static StaticQueue_t queue_control;
static unsigned char queue_storage[3 * sizeof(struct lc_message)];
static QueueHandle_t queue;
static StaticTask_t controls[3 + CR_EXTRA_TASK];
static StackType_t stacks[3 + CR_EXTRA_TASK][1024];
static TaskHandle_t tasks[3 + CR_EXTRA_TASK];
static atomic_uint stop, command, irq_after;
static unsigned received, sum, producer_done, consumer_done, fp_checks,
    monitor_checks;
static volatile unsigned nested, outer_msp, inner_msp, outer_lr, inner_lr,
    outer_psp;

static int allocate(int fault, const unsigned char *bytes, size_t count,
                    struct cr_token *token)
{
    taskENTER_CRITICAL();
    int result = cr_allocate(&pool, fault, bytes, count, token);
    taskEXIT_CRITICAL();
    return result;
}

static void release(struct cr_token token)
{
    taskENTER_CRITICAL();
    CR_CHECK(cr_release(&pool, token) == CR_OK);
    taskEXIT_CRITICAL();
}

static void offer(struct cr_token token)
{
    taskENTER_CRITICAL();
    CR_CHECK(cr_offer(&pool, token) == CR_OK);
    taskEXIT_CRITICAL();
}

static struct lc_message message(struct cr_token token)
{
    return (struct lc_message){pool.slots + token.slot, token};
}

static void receive(struct lc_message m)
{
    unsigned char bytes[16];
    size_t length = 0;
    CR_CHECK(m.token.slot < 3 && m.payload == pool.slots + m.token.slot);
    taskENTER_CRITICAL();
    CR_CHECK(cr_claim(&pool, m.token) == CR_OK);
    taskEXIT_CRITICAL();
    CR_CHECK(cr_read(&pool, m.token, bytes, sizeof bytes, &length) == CR_OK);
    for (size_t i = 0; i < length; i++)
        CR_CHECK(bytes[i] == (unsigned char)received);
    sum += (unsigned)length * (unsigned char)received;
    received++;
    release(m.token);
}

static void suspend_complete(unsigned which)
{
    if (which == 1)
        producer_done++;
    else
        consumer_done++;
    xTaskNotifyGive(tasks[0]);
    if (CR_TARGET_MUTANT != 2)
        vTaskSuspend(NULL);
}

static void raise_irq(void)
{
    *(volatile unsigned char *)0xe000e400u = CR_TARGET_MUTANT == 3 ? 96 : 160;
    *(volatile unsigned char *)0xe000e401u = 64;
    *(volatile unsigned *)0xe000e100u = 3;
    *(volatile unsigned *)0xe000e200u = 1;
    __asm__ volatile("dsb;isb" ::: "memory");
}

void cr_irq1_body(unsigned lr, unsigned msp, unsigned psp)
{
    (void)psp;
    inner_lr = lr;
    inner_msp = msp;
#if CR_FP
    unsigned bits = 0x40000000u;
    __asm__ volatile("vmov s0,%0" ::"r"(bits) : "s0");
#endif
    nested = 1;
}

void cr_irq0_body(unsigned lr, unsigned msp, unsigned psp)
{
    outer_lr = lr;
    outer_msp = msp;
    outer_psp = psp;
#if CR_FP
    unsigned bits = 0x3f800000u, after;
    __asm__ volatile("vmov s0,%0" ::"r"(bits) : "s0");
#endif
    *(volatile unsigned *)0xe000e200u = 2;
    __asm__ volatile("dsb;isb" ::: "memory");
    CR_CHECK(nested);
#if CR_FP
    __asm__ volatile("vmov %0,s0" : "=r"(after));
    CR_CHECK(after == bits);
#endif
    BaseType_t wake = pdFALSE;
    UBaseType_t mask = portSET_INTERRUPT_MASK_FROM_ISR();
    struct cr_token token;
    unsigned char byte = 201;
    CR_CHECK(cr_allocate(&pool, 1, &byte, 1, &token) == CR_OK);
    CR_CHECK(cr_offer(&pool, token) == CR_OK);
    portCLEAR_INTERRUPT_MASK_FROM_ISR(mask);
    struct lc_message m = message(token);
    CR_CHECK(xQueueSendFromISR(queue, &m, &wake) == pdPASS);
    portYIELD_FROM_ISR(wake);
}

static void producer(void *unused)
{
    (void)unused;
    for (;;)
    {
        CR_CHECK(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
        unsigned mode = atomic_load(&command);
        if (mode == 1)
        {
            unsigned char byte = 7;
            struct cr_token token;
            CR_CHECK(allocate(0, &byte, 1, &token) == CR_OK);
            offer(token);
            struct lc_message m = message(token);
            xTaskNotifyGive(tasks[0]);
            unsigned sent = 0;
            while (!atomic_load(&stop))
                if (xQueueSend(queue, &m, 10) == pdPASS)
                {
                    sent = 1;
                    break;
                }
            CR_CHECK(!sent);
            taskENTER_CRITICAL();
            CR_CHECK(cr_abort_offer(&pool, token) == CR_OK);
            taskEXIT_CRITICAL();
            if (CR_TARGET_MUTANT != 1)
                release(token);
            suspend_complete(1);
        }
        else if (mode == 2)
        {
            for (unsigned n = 0; n < 100; n++)
            {
                unsigned char bytes[16];
                size_t length = n % 16 + 1;
                for (size_t i = 0; i < length; i++)
                    bytes[i] = (unsigned char)n;
                struct cr_token token;
                int status;
                while ((status = allocate(0, bytes, length, &token)) == CR_FULL)
                    vTaskDelay(1);
                CR_CHECK(status == CR_OK);
                offer(token);
                struct lc_message m = message(token);
                while (xQueueSend(queue, &m, 1) != pdPASS)
                    CR_CHECK(!atomic_load(&stop));
#if CR_FP
                CR_CHECK(cr_fp_probe(0x3f800000u + n));
                taskENTER_CRITICAL();
                fp_checks++;
                taskEXIT_CRITICAL();
#endif
            }
            struct lc_message end = {0};
            CR_CHECK(xQueueSend(queue, &end, portMAX_DELAY) == pdPASS);
            suspend_complete(1);
        }
        else if (mode == 3)
        {
            raise_irq();
            atomic_store(&irq_after, 1);
            suspend_complete(1);
        }
        else
            cr_fail(__LINE__);
    }
}

static void consumer(void *unused)
{
    (void)unused;
    for (;;)
    {
        CR_CHECK(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
        for (;;)
        {
            struct lc_message m;
            CR_CHECK(xQueueReceive(queue, &m, portMAX_DELAY) == pdPASS);
            if (!m.payload)
                break;
            receive(m);
#if CR_FP
            CR_CHECK(cr_fp_probe(0x40400000u + received));
            taskENTER_CRITICAL();
            fp_checks++;
            taskEXIT_CRITICAL();
#endif
        }
        suspend_complete(2);
    }
}

static void quiescent(unsigned which)
{
    unsigned waits = 0;
    while (eTaskGetState(tasks[which]) != eSuspended)
    {
        CR_CHECK(waits++ < 4);
        vTaskDelay(1);
    }
}
#if CR_EXTRA_TASK
static void monitor_task(void *unused)
{
    (void)unused;
    CR_CHECK(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
    for (unsigned i = 0; i < 7; i++)
    {
        taskENTER_CRITICAL();
        unsigned owned = 0;
        for (unsigned j = 0; j < 3; j++)
            owned += pool.slots[j].state != CR_FREE &&
                     pool.slots[j].state != CR_RETIRED;
        CR_CHECK(owned <= 3);
        monitor_checks++;
        taskEXIT_CRITICAL();
    }
    xTaskNotifyGive(tasks[0]);
    vTaskSuspend(NULL);
    cr_fail(__LINE__);
}
#endif
static void controller(void *unused)
{
    (void)unused;
    unsigned char bytes[16] = {0};
    struct cr_token a, b, f, extra = {99, 99, 99};
    CR_CHECK(allocate(0, bytes, 16, &a) == CR_OK &&
             allocate(0, bytes, 16, &b) == CR_OK &&
             allocate(0, bytes, 16, &extra) == CR_FULL);
    CR_CHECK(allocate(1, bytes, 1, &f) == CR_OK && f.slot == 2);
    release(a);
    release(b);
    release(f);
    struct lc_message empty = {0};
    for (unsigned i = 0; i < 3; i++)
        CR_CHECK(xQueueSend(queue, &empty, 0) == pdPASS);
    atomic_store(&command, 1);
    xTaskNotifyGive(tasks[1]);
    CR_CHECK(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
    vTaskDelay(2);
    CR_CHECK(eTaskGetState(tasks[1]) == eBlocked);
    atomic_store(&stop, 1);
    CR_CHECK(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
    quiescent(1);
    CR_CHECK(pool.slots[0].state == CR_FREE && pool.slots[1].state == CR_FREE);
    for (unsigned i = 0; i < 3; i++)
        CR_CHECK(xQueueReceive(queue, &empty, 0) == pdPASS);
    cr_text("CR_SHUTDOWN:blocked-offer-reclaimed-once quiescent=1\n");
    atomic_store(&stop, 0);
    atomic_store(&command, 2);
    vTaskResume(tasks[1]);
    xTaskNotifyGive(tasks[1]);
    xTaskNotifyGive(tasks[2]);
#if CR_EXTRA_TASK
    xTaskNotifyGive(tasks[3]);
#endif
    unsigned done = 0;
    while (done < 2 + CR_EXTRA_TASK)
        done += (unsigned)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    CR_CHECK(done == 2 + CR_EXTRA_TASK);
    quiescent(1);
    quiescent(2);
#if CR_EXTRA_TASK
    quiescent(3);
    CR_CHECK(monitor_checks == 7);
#endif
    CR_CHECK(received == 100 && sum == 41780 &&
             uxQueueMessagesWaiting(queue) == 0);
    CR_CHECK(pool.slots[0].state == CR_FREE && pool.slots[1].state == CR_FREE);
#if CR_FP
    CR_CHECK(fp_checks == 200 && cr_fp_probe(0x40a00000u));
#else
    CR_CHECK(fp_checks == 0);
#endif
    atomic_store(&command, 3);
    atomic_store(&irq_after, 0);
    nested = 0;
    vTaskResume(tasks[1]);
    xTaskNotifyGive(tasks[1]);
    struct lc_message irq;
    CR_CHECK(xQueueReceive(queue, &irq, portMAX_DELAY) == pdPASS);
    CR_CHECK(!atomic_load(&irq_after));
    taskENTER_CRITICAL();
    CR_CHECK(cr_claim(&pool, irq.token) == CR_OK);
    taskEXIT_CRITICAL();
    size_t count = 0;
    CR_CHECK(cr_read(&pool, irq.token, bytes, 16, &count) == CR_OK &&
             count == 1 && bytes[0] == 201);
    release(irq.token);
    CR_CHECK(ulTaskNotifyTake(pdTRUE, portMAX_DELAY) == 1);
    quiescent(1);
    CR_CHECK(nested && inner_msp < outer_msp && outer_psp > 0);
#if CR_FP
    CR_CHECK((inner_lr & 16) == 0 && outer_msp - inner_msp >= 104);
#else
    CR_CHECK((inner_lr & 16) != 0 && outer_msp - inner_msp >= 32);
#endif
    cr_text("CR_CONTEXT:fp=");
    cr_number(CR_FP);
    cr_text(" task-checks=");
    cr_number(fp_checks);
    cr_text(" outer-lr=");
    cr_number(outer_lr);
    cr_text(" inner-lr=");
    cr_number(inner_lr);
    cr_text(" nested-MSP-delta=");
    cr_number(outer_msp - inner_msp);
    cr_text("\n");
    cr_text("CR_RESOURCES:application-stacks=");
    cr_number(sizeof stacks);
    cr_text(" task-controls=");
    cr_number(sizeof controls);
    cr_text(" queue-storage=");
    cr_number(sizeof queue_storage);
    cr_text(" queue-control=");
    cr_number(sizeof queue_control);
    cr_text(" payload-pool=");
    cr_number(sizeof pool);
    cr_text(" idle-stack-model=");
    cr_number(configMINIMAL_STACK_SIZE * sizeof(StackType_t));
    cr_text(" watermark0=");
    cr_number(uxTaskGetStackHighWaterMark(tasks[0]) * sizeof(StackType_t));
    cr_text("\n");
    CR_CHECK(cr_close(&pool) == CR_OK && cr_drained(&pool) == CR_OK &&
             producer_done == 3 && consumer_done == 1);
    vTaskDelete(tasks[1]);
    vTaskDelete(tasks[2]);
#if CR_EXTRA_TASK
    vTaskDelete(tasks[3]);
#endif
    vQueueDelete(queue);
    cr_text(
        "CR_WORK:count=100 sum=41780 reserved-fault=1 isr-handoff=1 monitor-checks=");
    cr_number(monitor_checks);
    cr_text("\n");
    cr_done();
}

void cr_run(void)
{
    CR_CHECK(cr_pool_init(&pool, 1) == CR_OK);
    atomic_init(&stop, 0);
    atomic_init(&command, 0);
    atomic_init(&irq_after, 0);
    queue = xQueueCreateStatic(3, sizeof(struct lc_message), queue_storage,
                               &queue_control);
    CR_CHECK(queue);
    tasks[0] = xTaskCreateStatic(controller, "control", 1024, NULL, 4,
                                 stacks[0], &controls[0]);
    tasks[1] = xTaskCreateStatic(producer, "producer", 1024, NULL, 1, stacks[1],
                                 &controls[1]);
    tasks[2] = xTaskCreateStatic(consumer, "consumer", 1024, NULL, 2, stacks[2],
                                 &controls[2]);
    CR_CHECK(tasks[0] && tasks[1] && tasks[2]);
#if CR_EXTRA_TASK
    tasks[3] = xTaskCreateStatic(monitor_task, "monitor", 1024, NULL, 3,
                                 stacks[3], &controls[3]);
    CR_CHECK(tasks[3]);
#endif
    vTaskStartScheduler();
    cr_fail(__LINE__);
}
