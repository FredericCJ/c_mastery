/* Reuse the already inspected F439 setup and strict providers explicitly.
   Its polling/DMA demonstration is retained under renamed entry points. */
#define cp_target_main p4_legacy_main
#define cp_pendsv p4_legacy_pendsv
#include "../checkpoint_depth/targets/f439.c"
#undef cp_target_main
#undef cp_pendsv
#include "capture_layout.h"
#include "halfduplex.h"
#include "register_rules.h"

/*
 * F439 capture experiment: EXTI0 (PA0) timestamps each edge into a ring; the
 * foreground drains the ring and sends each sample on USART2, by polling
 * (P4_MODE 0), DMA1 stream 6 (1) or the half-duplex component (2). Rows of
 * p4_capture record DWT cycle counts at each stage when P4_TRACE is 1. The
 * image is only cross-built and audited on the host.
 *
 * Ownership: the EXTI0 handler writes a row's acquisition fields; the
 * foreground writes the later fields of the same row, one row per sequence.
 * `received` is written only by the handler; the foreground reads it with
 * interrupts masked (received_and_dropped).
 */
#ifndef P4_TRACE
#define P4_TRACE 1
#endif
#ifndef P4_MODE
#define P4_MODE 1
#endif
#ifndef P4_RING
#define P4_RING 4
#endif

/* Cortex-M4 core registers. */
#define DEMCR UINT32_C(0xe000edfc)
#define DWT_CTRL UINT32_C(0xe0001000)
#define DWT_CYCCNT UINT32_C(0xe0001004)
#define NVIC_ISER0 UINT32_C(0xe000e100)
#define NVIC_ICPR0 UINT32_C(0xe000e280)
#define NVIC_IPR_EXTI0 UINT32_C(0xe000e406)

/* STM32F439 peripherals (RM0090). */
#define RCC_AHB1ENR UINT32_C(0x40023830)
#define RCC_APB2ENR UINT32_C(0x40023844)
#define SYSCFG_EXTICR1 UINT32_C(0x40013808)
#define EXTI_IMR UINT32_C(0x40013c00)
#define EXTI_EMR UINT32_C(0x40013c04)
#define EXTI_RTSR UINT32_C(0x40013c08)
#define EXTI_FTSR UINT32_C(0x40013c0c)
#define EXTI_PR UINT32_C(0x40013c14)
#define GPIOA_MODER UINT32_C(0x40020000)
#define GPIOA_OTYPER UINT32_C(0x40020004)
#define GPIOA_OSPEEDR UINT32_C(0x40020008)
#define GPIOA_PUPDR UINT32_C(0x4002000c)
#define GPIOA_BSRR UINT32_C(0x40020018)
#define GPIOC_MODER UINT32_C(0x40020800)
#define GPIOC_OTYPER UINT32_C(0x40020804)
#define GPIOC_OSPEEDR UINT32_C(0x40020808)
#define GPIOC_PUPDR UINT32_C(0x4002080c)
#define GPIOC_BSRR UINT32_C(0x40020818)
#define USART2_SR UINT32_C(0x40004400)
#define USART2_DR UINT32_C(0x40004404)
#define USART2_CR1 UINT32_C(0x4000440c)
#define USART2_CR3 UINT32_C(0x40004414)
#define DMA1_HISR UINT32_C(0x40026004)
#define DMA1_HIFCR UINT32_C(0x4002600c)
#define DMA1_S6CR UINT32_C(0x400260a0)
#define DMA1_S6NDTR UINT32_C(0x400260a4)
#define DMA1_S6PAR UINT32_C(0x400260a8)
#define DMA1_S6M0AR UINT32_C(0x400260ac)
#define DMA1_S6FCR UINT32_C(0x400260b4)

/* Register bit fields used below. */
#define USART_SR_TC UINT32_C(64)
#define USART_SR_TXE UINT32_C(128)
#define USART_CR1_UE (UINT32_C(1) << 13)
#define USART_CR3_DMAT UINT32_C(0x80)
#define DMA_S6CR_EN UINT32_C(1)
/* HIFCR: clear every stream-6 flag (FE, DME, TE, HT, TC). */
#define DMA_HIFCR_CLEAR_S6 (UINT32_C(0x3d) << 16)
/* HISR: stream-6 error flags (FE, DME, TE). */
#define DMA_HISR_ERRORS_S6 (UINT32_C(0xd) << 16)
/* CR: channel 4, memory increment, memory-to-peripheral. */
#define DMA_S6CR_SETUP \
    ((UINT32_C(4) << 25) | (UINT32_C(1) << 10) | (UINT32_C(1) << 6))
#define EXTI_LINE0 UINT32_C(1)
#define EXTI0_IRQ 6

/* Timing budgets. Cycle counts are DWT cycles at the 16 MHz HSI clock. */
enum
{
    POLL_BUDGET = 1000000, /* loop iterations per frame; see send() */
    FRAME_TIMEOUT_CYCLES = 1600000, /* 100 ms */
    DIRECTION_HOLD_CYCLES = 16000, /* nominal 1 ms after the TC marker */
    CAPTURE_WINDOW_CYCLES = 32000000, /* 2 s overall */
    TURNAROUND_TICKS = 1000, /* 16-cycle ticks; 1000 = 1 ms at 16 MHz */
    EXTI0_PRIORITY = 128,
    PAYLOAD_BYTES = 8
};

/* Values of p4_capture.state. */
enum
{
    CAPTURE_RUNNING = 1,
    CAPTURE_STOPPED = 2,
    CAPTURE_FAILED = 3
};

/* Values of p4_capture.first_fault (the first fault is never overwritten). */
enum
{
    FAULT_DMA_ERROR = 1, /* DMA FE/DME/TE error, returned by cpf_tx_dma */
    FAULT_TIMEOUT = 2, /* frame deadline, poll budget or busy DMA stream */
    FAULT_BEGIN_REFUSED = 3, /* transmitter was not idle at the next frame */
    FAULT_NEXT_FAILED = 4 /* frame done but the next one could not start */
};

/*
 * PA3..PA7 are scope markers. PA4..PA7 are pulsed once per event stage; PA3
 * is held high for the body of the EXTI0 handler.
 */
enum
{
    MARKER_ISR = 3,
    MARKER_PUBLISH = 4,
    MARKER_WIRE_DONE = 5,
    MARKER_FRAMED = 6,
    MARKER_DMA_DONE = 7
};

volatile struct p4_capture p4_capture;
static volatile uint32_t initialized = UINT32_C(0x13579bdf);
static volatile uint32_t zeroed;
static struct cpx_half_duplex half;
static volatile unsigned received;
static unsigned release_requested;

static uint32_t now(void)
{
    return REG(DWT_CYCCNT);
}

/* Consistent pair read: the handler cannot run between the two loads. */
static unsigned received_and_dropped(unsigned *drops)
{
    unsigned saved;
    __asm__ volatile("mrs %0,primask\ncpsid i" : "=r"(saved)::"memory");
    unsigned count = received;
    *drops = ring.dropped;
    __asm__ volatile("msr primask,%0" ::"r"(saved) : "memory");
    return count;
}

/* Drives marker pin `bit` of GPIOA high (`set`) or low through BSRR. */
static void marker(unsigned bit, int set)
{
    if (P4_TRACE)
    {
        REG(GPIOA_BSRR) = UINT32_C(1) << (bit + (set ? 0 : 16));
    }
}

static void pulse(unsigned bit)
{
    marker(bit, 1);
    marker(bit, 0);
}

static void direction(void *unused, int on)
{
    (void)unused;
    if (on)
    {
        release_requested = 0;
        REG(GPIOC_BSRR) = 1;
    }
    else
    {
        /* Hardware adapter releases after its own cycle guard. */
        release_requested = 1;
    }
}

static void release_direction(void)
{
    REG(GPIOC_BSRR) = UINT32_C(1) << 16;
    release_requested = 0;
}

/* Enables the DWT cycle counter; returns 0 if it is absent or not counting. */
static int clock_setup(void)
{
    REG(DEMCR) |= UINT32_C(1) << 24;
    __asm__ volatile("dsb\nisb" ::: "memory");
    if (REG(DWT_CTRL) & (UINT32_C(1) << 25))
    {
        return 0; /* NOCYCCNT: no cycle counter on this part */
    }
    REG(DWT_CYCCNT) = 0;
    REG(DWT_CTRL) |= 1;
    uint32_t before = now();
    for (unsigned i = 0; i < 100; i++)
    {
        __asm__ volatile("nop");
    }
    return now() != before;
}

void cp_pendsv(void)
{
    cp_fault();
}

/* PA0 edge: timestamp, publish to the ring, count. Bounded, no waiting. */
void p4_exti0(void)
{
    if (!(REG(EXTI_PR) & EXTI_LINE0))
    {
        return;
    }
    /* W1C, never read-modify-write unrelated pending bits. */
    REG(EXTI_PR) = EXTI_LINE0;
    unsigned index = received;
    if (index >= P4_EVENTS)
    {
        REG(EXTI_IMR) &= ~EXTI_LINE0;
        return;
    }
    marker(MARKER_ISR, 1);
    uint32_t tick = now();
    volatile struct p4_row *row = &p4_capture.rows[index];
    row->sequence = index;
    if (P4_TRACE)
    {
        row->isr_cycle = tick;
    }
    int accepted =
        cpf_irq_push(&ring, (struct cpf_sample){index, (uint16_t)(100 + index),
                                                (uint16_t)(tick >> 4)});
    if (P4_TRACE)
    {
        row->publish_cycle = now();
    }
    row->flags = accepted ? P4_ACCEPTED : P4_DROPPED;
    pulse(MARKER_PUBLISH);
    marker(MARKER_ISR, 0);
    received = index + 1;
    if (received == P4_EVENTS)
    {
        REG(EXTI_IMR) &= ~EXTI_LINE0;
    }
}

static void inputs_setup(void)
{
    REG(RCC_AHB1ENR) |=
        UINT32_C(4); /* GPIOC for declared teaching direction PC0. */
    REG(RCC_APB2ENR) |= UINT32_C(1) << 14; /* SYSCFG */
    (void)REG(RCC_APB2ENR);
    __asm__ volatile("dsb" ::: "memory");
    /* PA0 input, no pull. PA3..7 output markers; PA2 remains USART2 AF7. */
    REG(GPIOA_MODER) =
        (REG(GPIOA_MODER) & ~UINT32_C(0xffc3)) | UINT32_C(0x5540);
    REG(GPIOA_OTYPER) &= ~UINT32_C(0xf8);
    REG(GPIOA_OSPEEDR) &= ~UINT32_C(0xffc0);
    REG(GPIOA_PUPDR) &= ~UINT32_C(0xffc3);
    REG(GPIOA_BSRR) = UINT32_C(0xf8) << 16;
    REG(GPIOC_MODER) = (REG(GPIOC_MODER) & ~UINT32_C(3)) | 1;
    REG(GPIOC_OTYPER) &= ~UINT32_C(1);
    REG(GPIOC_OSPEEDR) &= ~UINT32_C(3);
    REG(GPIOC_PUPDR) &= ~UINT32_C(3);
    release_direction();
    REG(SYSCFG_EXTICR1) &= ~UINT32_C(15); /* EXTI0 explicitly selects PA0. */
    REG(EXTI_IMR) &= ~EXTI_LINE0;
    REG(EXTI_EMR) &= ~EXTI_LINE0;
    REG(EXTI_RTSR) |= EXTI_LINE0;
    REG(EXTI_FTSR) &= ~EXTI_LINE0;
    REG(EXTI_PR) = EXTI_LINE0;
    *(volatile uint8_t *)NVIC_IPR_EXTI0 = EXTI0_PRIORITY;
    REG(NVIC_ICPR0) = UINT32_C(1) << EXTI0_IRQ;
    REG(NVIC_ISER0) = UINT32_C(1) << EXTI0_IRQ;
    REG(EXTI_IMR) |= EXTI_LINE0;
}

/* Packs sequence (32 bits), value (16) and tick (16), little-endian. */
static void fill_message(struct cp_message *message,
                         const struct cpf_sample *sample)
{
    *message = (struct cp_message){0};
    message->sequence = (uint16_t)sample->sequence;
    message->length = PAYLOAD_BYTES;
    for (unsigned i = 0; i < 4; i++)
    {
        message->payload[i] = (unsigned char)(sample->sequence >> (8 * i));
    }
    message->payload[4] = (unsigned char)sample->value;
    message->payload[5] = (unsigned char)(sample->value >> 8);
    message->payload[6] = (unsigned char)sample->tick;
    message->payload[7] = (unsigned char)(sample->tick >> 8);
}

/* Stores the first-fault snapshot for a transfer that stopped the run. */
static void record_fault(volatile struct p4_row *row,
                         const struct cpf_tx *owner, unsigned code)
{
    row->flags |= P4_FAILED;
    p4_capture.first_fault = code;
    p4_capture.fault_generation = owner->generation;
    p4_capture.retained = (uint32_t)owner->retained;
}

/*
 * Programs DMA1 stream 6 to copy the framed bytes to USART2->DR and starts it.
 * Returns 0, leaving the hardware untouched, if the stream is still enabled
 * from an earlier transfer.
 */
static int dma_start(void)
{
    if (REG(DMA1_S6CR) & DMA_S6CR_EN)
    {
        return 0;
    }
    REG(DMA1_HIFCR) = DMA_HIFCR_CLEAR_S6;
    REG(DMA1_S6CR) = DMA_S6CR_SETUP;
    REG(DMA1_S6FCR) = 0; /* direct mode, byte transfers */
    REG(DMA1_S6NDTR) = (uint32_t)tx.length;
    REG(DMA1_S6PAR) = USART2_DR;
    REG(DMA1_S6M0AR) = (uint32_t)(uintptr_t)tx.wire;
    REG(USART2_SR) = p4_tc_clear_word();
    __asm__ volatile("dmb" ::: "memory");
    REG(USART2_CR3) |= USART_CR3_DMAT;
    REG(DMA1_S6CR) |= DMA_S6CR_EN;
    return 1;
}

/* DMA quiescent (EN=0, NDTR=0) is the "all bytes moved" mark, recorded once. */
static void note_dma_done(volatile struct p4_row *row, unsigned enabled,
                          uint32_t remaining)
{
    if (!enabled && !remaining && !(row->flags & P4_DMA_DONE))
    {
        row->flags |= P4_DMA_DONE;
        if (P4_TRACE)
        {
            row->dma_cycle = now();
        }
        pulse(MARKER_DMA_DONE);
    }
}

/* One look at the hardware; advances the transfer state machine. */
static enum cpf_status poll_transfer(volatile struct p4_row *row,
                                     struct cpf_tx *active, uint32_t status)
{
    if (P4_MODE == 1)
    {
        unsigned enabled = REG(DMA1_S6CR) & DMA_S6CR_EN;
        uint32_t remaining = REG(DMA1_S6NDTR);
        uint32_t flags = REG(DMA1_HISR);
        note_dma_done(row, enabled, remaining);
        return cpf_tx_dma(active, active->generation, (int)enabled, remaining,
                          (status & USART_SR_TC) != 0,
                          (flags & DMA_HISR_ERRORS_S6) != 0);
    }
    if (P4_MODE == 2)
    {
        return release_requested
                   ? CPF_OK
                   : cpx_halfduplex_step(&half, (uint16_t)(now() >> 4),
                                         (status & USART_SR_TXE) != 0,
                                         (status & USART_SR_TC) != 0,
                                         write_byte, 0);
    }
    return cpf_tx_poll(active, active->generation, (status & USART_SR_TXE) != 0,
                       (status & USART_SR_TC) != 0, write_byte, 0);
}

/*
 * Records the first TC observation of a frame. TC is a sampled status
 * observation, not the exact external stop-bit edge. Returns the DWT value at
 * which it was seen, or `previous` if this call saw nothing new.
 */
static uint32_t note_wire_done(volatile struct p4_row *row, int complete,
                               uint32_t previous)
{
    if (complete && !(row->flags & P4_WIRE_DONE))
    {
        row->flags |= P4_WIRE_DONE;
        if (P4_TRACE)
        {
            row->tc_cycle = now();
        }
        pulse(MARKER_WIRE_DONE);
        return now();
    }
    return previous;
}

/*
 * Abort path after a DMA error, a frame timeout, a busy DMA stream at start or
 * an exhausted poll budget (for example a frozen clock): stop DMA and the
 * request, wait for acknowledgement, and only then let the state machine
 * recover. Leaves a first-fault snapshot (the transmitter's own code if it
 * already holds one, else FAULT_TIMEOUT); never replays a partial frame.
 * The acknowledgement wait is bounded by wait_mask (at most 1,000,000 reads of
 * DMA1_S6CR).
 */
static void abort_transfer(volatile struct p4_row *row, struct cpf_tx *active)
{
    (void)cpf_tx_timeout(active, active->generation, FAULT_TIMEOUT);
    row->flags |= P4_FAILED;
    REG(USART2_CR3) &= ~USART_CR3_DMAT;
    REG(DMA1_S6CR) &= ~DMA_S6CR_EN;
    if (wait_mask(DMA1_S6CR, DMA_S6CR_EN, 0))
    {
        REG(USART2_CR1) &= ~USART_CR1_UE;
        (void)cpf_tx_recover(active, active->generation, 0,
                             (REG(USART2_CR1) & USART_CR1_UE) != 0);
        if (!active->retained && P4_MODE == 2)
        {
            release_direction();
        }
    }
    p4_capture.first_fault =
        active->fault_code ? active->fault_code : FAULT_TIMEOUT;
    p4_capture.fault_generation =
        active->fault_code ? active->fault_generation : active->generation;
    p4_capture.retained = (uint32_t)active->retained;
    p4_capture.retries = active->retries;
}

/* Half duplex: the bus is held for a guard time after the TC marker. */
static int direction_may_release(uint32_t tc_observed)
{
    return release_requested &&
           (uint32_t)(now() - tc_observed) >= DIRECTION_HOLD_CYCLES;
}

/*
 * The frame completed. Half duplex releases the bus first; then DMA requests
 * are disabled and, in modes 0 and 1, the next frame is armed. Returns 1 to
 * continue, 0 if the next frame could not be armed (first fault recorded).
 */
static int finish_frame(volatile struct p4_row *row, struct cpf_tx *active)
{
    if (P4_MODE == 2)
    {
        release_direction();
    }
    REG(USART2_CR3) &= ~USART_CR3_DMAT;
    if (P4_MODE == 2 || cpf_tx_next(active) == CPF_OK)
    {
        return 1;
    }
    record_fault(row, active, FAULT_NEXT_FAILED);
    return 0;
}

/*
 * Sends one sample and waits for the frame to finish, bounded by
 * POLL_BUDGET loop iterations and FRAME_TIMEOUT_CYCLES of DWT time. Each
 * iteration reads USART2_SR and the DWT counter, plus three DMA registers in
 * mode 1.
 * Returns 1 on success. Returns 0 after recording a first fault; the caller
 * must stop the experiment (no automatic partial-frame replay).
 */
static int send(const struct cpf_sample *sample)
{
    struct cp_message message;
    fill_message(&message, sample);
    volatile struct p4_row *row = &p4_capture.rows[sample->sequence];
    struct cpf_tx *active = P4_MODE == 2 ? &half.tx : &tx;
    enum cpf_status begun =
        P4_MODE == 2 ? cpx_halfduplex_begin(&half, &message, TURNAROUND_TICKS,
                                            direction, 0)
                     : cpf_tx_begin(&tx, &message, P4_MODE == 1);
    if (begun != CPF_OK)
    {
        /* Refused precondition: do not reset an unknown owner. */
        record_fault(row, active, FAULT_BEGIN_REFUSED);
        return 0;
    }
    row->generation = active->generation;
    row->flags |= P4_FRAMED;
    if (P4_TRACE)
    {
        row->frame_cycle = now();
    }
    pulse(MARKER_FRAMED);
    if (P4_MODE == 1 && !dma_start())
    {
        abort_transfer(row, active);
        return 0;
    }
    uint32_t start = now();
    uint32_t tc_observed = 0;
    for (unsigned budget = 0; budget < POLL_BUDGET; budget++)
    {
        uint32_t status = REG(USART2_SR);
        enum cpf_status result = poll_transfer(row, active, status);
        int complete = P4_MODE == 2 ? half.holding : result == CPF_OK;
        tc_observed = note_wire_done(row, complete, tc_observed);
        if (result == CPF_OK)
        {
            if (P4_MODE == 2 && !direction_may_release(tc_observed))
            {
                continue;
            }
            return finish_frame(row, active);
        }
        if (result == CPF_FAILED ||
            (uint32_t)(now() - start) >= FRAME_TIMEOUT_CYCLES)
        {
            break;
        }
    }
    abort_transfer(row, active);
    return 0;
}

void cp_target_main(void)
{
    if (initialized != UINT32_C(0x13579bdf) || zeroed || received ||
        p4_capture.magic)
    {
        cp_fault();
    }
    if (!cpf_ring_init(&ring, P4_RING) || !setup() || !clock_setup())
    {
        cp_fault();
    }
    cpf_tx_init(&tx, 128, 2);
    p4_capture.magic = P4_CAPTURE_MAGIC;
    p4_capture.version = 1;
    p4_capture.trace_enabled = P4_TRACE;
    p4_capture.mode = P4_MODE;
    p4_capture.ring_capacity = P4_RING;
    p4_capture.state = CAPTURE_RUNNING;
    p4_capture.clock_start = now();
    inputs_setup();
    __asm__ volatile("cpsie i" ::: "memory");
    unsigned completed = 0;
    int failed = 0;
    while ((uint32_t)(now() - p4_capture.clock_start) < CAPTURE_WINDOW_CYCLES)
    {
        struct cpf_sample sample;
        if (cpf_take(&ring, &sample))
        {
            if (!send(&sample))
            {
                failed = 1;
                break;
            }
            ++completed;
        }
        unsigned dropped;
        if (received_and_dropped(&dropped) == P4_EVENTS &&
            completed + dropped == P4_EVENTS)
        {
            break;
        }
    }
    __asm__ volatile("cpsid i" ::: "memory");
    REG(EXTI_IMR) &= ~EXTI_LINE0;
    p4_capture.clock_end = now();
    p4_capture.received = received;
    p4_capture.dropped = ring.dropped;
    p4_capture.accepted = received - ring.dropped;
    p4_capture.completed = completed;
    p4_capture.state = failed ? CAPTURE_FAILED : CAPTURE_STOPPED;
    __asm__ volatile("dsb" ::: "memory");
    for (;;)
    {
    }
}
