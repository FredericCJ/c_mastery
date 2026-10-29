#include "checkpoint_firmware.h"
#define REG(address) (*(volatile uint32_t *)(uintptr_t)(address))
/* Dedicated STM32F439, reset HSI profile, PA2 AF7 USART2 TX, DMA1 stream6/channel4.
 * No driver executes on the host. The runner builds/audits but never flashes it. */
static struct cpf_ring ring;
static struct cpf_tx tx __attribute__((section(".dma"), aligned(4)));
static volatile unsigned calls;
volatile uint32_t cp_board_fault;

void cp_fault(void)
{
    cp_board_fault = UINT32_C(0xdead0001);
    __asm__ volatile("cpsid i" ::: "memory");
    for (;;)
    {
    }
}

void cp_pendsv(void)
{
    unsigned n = calls;
    (void)cpf_irq_push(
        &ring, (struct cpf_sample){n, (uint16_t)(100 + n), (uint16_t)n});
    calls = n + 1;
}

static int wait_mask(uintptr_t address, uint32_t mask, uint32_t value)
{
    for (unsigned n = 0; n < 1000000; ++n)
        if ((REG(address) & mask) == value)
            return 1;
    return 0;
}

static void write_byte(void *context, unsigned char byte)
{
    (void)context;
    REG(0x40004404) = byte;
}

static int setup(void)
{
    REG(0x40023800) |= 1; /* HSION, then require HSIRDY. */
    if (!wait_mask(0x40023800, 2, 2))
        return 0;
    REG(0x40023808) &=
        ~UINT32_C(3); /* Select HSI before changing prescalers. */
    if (!wait_mask(0x40023808, 12, 0))
        return 0;
    REG(0x40023808) &= ~UINT32_C(0xfcf0); /* AHB/APB1/APB2 divide by1. */
    REG(0x40023830) |= UINT32_C(1) | (UINT32_C(1) << 21); /* GPIOA, DMA1. */
    REG(0x40023840) |= UINT32_C(1) << 17; /* USART2. */
    (void)REG(0x40023840);
    __asm__ volatile("dsb" ::: "memory");
    REG(0x40020000) = (REG(0x40020000) & ~UINT32_C(0x30)) | UINT32_C(0x20);
    REG(0x40020004) &= ~UINT32_C(4); /* PA2 push-pull. */
    REG(0x40020008) &=
        ~UINT32_C(0x30); /* Low speed; loading remains board obligation. */
    REG(0x4002000c) &= ~UINT32_C(0x30);
    REG(0x40020020) = (REG(0x40020020) & ~UINT32_C(0xf00)) | UINT32_C(0x700);
    REG(0x4000440c) = 0;
    REG(0x40004410) = 0;
    REG(0x40004414) = 0;
    REG(0x40004408) =
        139; /* 16MHz/(16*USARTDIV), OVER8=0: actual 115107.9baud. */
    REG(0x4000440c) = (UINT32_C(1) << 13) | (UINT32_C(1) << 3);
    return 1;
}

static int transfer(const struct cp_message *message, int dma)
{
    if (cpf_tx_begin(&tx, message, dma) != CPF_OK)
        return 0;
    if (dma)
    {
        if (REG(0x400260a0) & 1)
            return 0;
        REG(0x4002600c) = UINT32_C(0x3d)
                          << 16; /* Stream6 flags, write-one-to-clear. */
        REG(0x400260a0) =
            (UINT32_C(4) << 25) | (UINT32_C(1) << 10) | (UINT32_C(1) << 6);
        REG(0x400260b4) = 0; /* Direct mode, byte transfers. */
        REG(0x400260a4) = (uint32_t)tx.length;
        REG(0x400260a8) = UINT32_C(0x40004404);
        REG(0x400260ac) = (uint32_t)(uintptr_t)tx.wire;
        REG(0x40004400) = ~UINT32_C(
            0x40); /* Clear stale TC for this multibuffer generation. */
        __asm__ volatile("dmb" ::: "memory");
        REG(0x40004414) |= UINT32_C(0x80);
        REG(0x400260a0) |= 1;
    }
    for (unsigned budget = 0; budget < 1000000; ++budget)
    {
        uint32_t status = REG(0x40004400);
        enum cpf_status result;
        if (dma)
        {
            uint32_t flags = REG(0x40026004);
            result = cpf_tx_dma(&tx, tx.generation, (REG(0x400260a0) & 1) != 0,
                                REG(0x400260a4), (status & 64) != 0,
                                (flags & (UINT32_C(0xd) << 16)) != 0);
        }
        else
            result = cpf_tx_poll(&tx, tx.generation, (status & 128) != 0,
                                 (status & 64) != 0, write_byte, 0);
        if (result == CPF_OK)
        {
            REG(0x40004414) &= ~UINT32_C(0x80);
            return cpf_tx_next(&tx) == CPF_OK;
        }
        if (result == CPF_FAILED)
            break;
    }
    (void)cpf_tx_timeout(&tx, tx.generation, 2);
    REG(0x40004414) &= ~UINT32_C(0x80);
    REG(0x400260a0) &= ~UINT32_C(1);
    /* The disable request alone is insufficient: failed acknowledgement retains tx. */
    if (!wait_mask(0x400260a0, 1, 0))
        return 0;
    REG(0x4000440c) &= ~(UINT32_C(1) << 13);
    if (cpf_tx_recover(&tx, tx.generation, 0,
                       (REG(0x4000440c) & (UINT32_C(1) << 13)) != 0) != CPF_OK)
        return 0;
    return 0; /* Report failed partial frame; never silently replay it. */
}

void cp_target_main(void)
{
    if (!cpf_ring_init(&ring, 4) || !setup())
        cp_fault();
    cpf_tx_init(&tx, 16, 2);
    *(volatile uint8_t *)UINT32_C(0xe000ed22) = 255;
    __asm__ volatile("cpsie i" ::: "memory");
    for (unsigned i = 0; i < 5; ++i)
    {
        REG(0xe000ed04) = UINT32_C(1) << 28;
        __asm__ volatile("dsb\nisb" ::: "memory");
        while (calls != i + 1)
        {
        }
    }
    struct cpf_sample sample;
    unsigned selected = 0;
    while (cpf_take(&ring, &sample))
    {
        struct cp_message message = {
            .sequence = (uint16_t)sample.sequence,
            .length = 4,
            .payload = {
                (unsigned char)sample.value, (unsigned char)(sample.value >> 8),
                (unsigned char)sample.tick, (unsigned char)(sample.tick >> 8)}};
        if (!transfer(&message, selected++ != 0))
            cp_fault();
    }
    cp_board_fault = 0;
    for (;;)
    {
        __asm__ volatile("wfi");
    }
}
