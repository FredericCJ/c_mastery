#include "board.h"
#include "board_input.h"
#include "stm32f4xx_hal.h"

/* Dedicated teaching-board profile only. Already running HSI reset clock,
 * exclusive RCC/GPIOA configuration owner; PA2 selected free teaching pin. */
static uint32_t read_register(void *ctx, uintptr_t address)
{
    (void)ctx;
    return *(volatile uint32_t *)address;
}

static void write_register(void *ctx, uintptr_t address, uint32_t value)
{
    (void)ctx;
    *(volatile uint32_t *)address = value;
}

static void clock_gpio(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    (void)RCC->AHB1ENR;
    __DSB();
}

int ar_target_minimal(int32_t milli)
{
    clock_gpio();
    struct ar_board board = {0, read_register, write_register, GPIOA_BASE, 2,
                             0};
    if (ar_board_init(&board) != AR_OK)
        return AR_FAILED;
    return ar_service(milli, 100, (struct ar_indicator){&board, ar_board_set});
}

int ar_target_vendor(int32_t milli)
{
    clock_gpio();
    GPIO_InitTypeDef c = {GPIO_PIN_2, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL,
                          GPIO_SPEED_FREQ_LOW, 0};
    HAL_GPIO_Init(GPIOA, &c);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2,
                      milli >= 100 ? GPIO_PIN_SET : GPIO_PIN_RESET);
    return AR_OK;
}

static uint32_t uart_read(void *ctx, uint32_t address)
{
    return read_register(ctx, address);
}

static void uart_write(void *ctx, uint32_t address, uint32_t value)
{
    write_register(ctx, address, value);
}

static void uart_sync(void *ctx)
{
    (void)ctx;
    __DSB();
    __ISB();
}

static struct fh_io uart_bus(void)
{
    return (struct fh_io){0, uart_read, uart_write, uart_sync};
}

int ar_target_input_init(void)
{
    struct fh_io bus = uart_bus();
    return f4_cold(bus) && f4_uart_init(bus) ? AR_OK : AR_FAILED;
}

struct ar_result ar_target_input(unsigned char *p, size_t n)
{
    return ar_f439_read(uart_bus(), p, n);
}
