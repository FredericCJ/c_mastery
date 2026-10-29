/* Import the frozen original P4 port in this translation unit so its exact
 * bounded setup/transfer routines are composed, not reimplemented by name.
 * Its original entry remains separately named and is discarded by section GC.
 */
#define cp_target_main p4_retained_entry
#include "../../checkpoint_depth/targets/f439.c"
#undef cp_target_main
#include "firmware.h"
static struct cf_firmware cap_board
    __attribute__((section(".dma"), aligned(32)));
static struct pd_port port;

static int hsi_ready(void *p)
{
    (void)p;
    return (REG(0x40023800) & 2u) != 0;
}

void cp_target_main(void)
{
    if (!cf_init(&cap_board) || !setup() ||
        cf_boot(&cap_board, hsi_ready, 0, 32) != CF_OK ||
        !cpf_ring_init(&ring, 4))
        cp_fault();
    cpf_tx_init(&tx, 16, 2);
    struct pd_io io = pd_target_io(&port);
    *(volatile uint8_t *)UINT32_C(0xe000ed22) = 255;
    __asm__ volatile("cpsie i" ::: "memory");
    for (unsigned i = 0; i < 5; ++i)
    {
        REG(0xe000ed04) = UINT32_C(1) << 28;
        __asm__ volatile("dsb\nisb" ::: "memory");
        unsigned budget = 1000000;
        while (calls != i + 1 && budget)
            --budget;
        if (!budget)
            cp_fault();
    }
    struct cpf_sample sample;
    unsigned token = 0;
    while (cpf_take(&ring, &sample))
    {
        for (unsigned i = 0; i < 4; ++i)
            cap_board.dma_source[i] =
                (unsigned char)(sample.sequence >> (8 * i));
        cap_board.dma_source[4] = (unsigned char)sample.value;
        cap_board.dma_source[5] = (unsigned char)(sample.value >> 8);
        cap_board.dma_source[6] = (unsigned char)sample.tick;
        cap_board.dma_source[7] = (unsigned char)(sample.tick >> 8);
        if (cf_dma_begin(&cap_board, io,
                         (uint32_t)(uintptr_t)cap_board.dma_source,
                         (uint32_t)(uintptr_t)cap_board.dma_destination,
                         ++token) != CF_OK)
            cp_fault();
        enum cf_status completion = CF_PENDING;
        for (unsigned budget = 0; budget < 1000000 && completion == CF_PENDING;
             ++budget)
            completion = cf_dma_finish(&cap_board, io, token);
        if (completion != CF_OK)
        {
            (void)cf_dma_cancel(&cap_board, io, token);
            cf_fault(&cap_board, 31);
            cp_fault();
        } /* Retain/quarantine on unacknowledged timeout. */
        if (cf_foreground(&cap_board, 1) != CF_OK ||
            cs_begin(&cap_board.core) != CS_OK)
            cp_fault();
        const unsigned char *wire;
        size_t length;
        unsigned generation;
        if (cs_lease(&cap_board.core, &wire, &length, &generation) != CS_OK)
            cp_fault();
        struct cp_parser parser;
        cp_parser_init(&parser);
        size_t used = 0;
        struct cp_message message;
        if (cp_feed(&parser, wire, length, &used) != CP_FRAME ||
            used != length || cp_peek(&parser, &message) != CP_OK)
            cp_fault();
        if (!transfer(&message, token != 1))
        {
            cf_fault(&cap_board, 32);
            cp_fault();
        } /* P4 waits selected DMA quiescence and UART TC. */
        if (cs_complete(&cap_board.core, generation, 1, length, 0, 0) !=
                CS_DONE ||
            !cf_watchdog(&cap_board, 3))
            cp_fault();
    }
    if (cap_board.core.delivered != 3 || ring.dropped != 2 ||
        cap_board.feeds != 3)
        cp_fault();
    cp_board_fault = 0;
    for (;;)
        __asm__ volatile("wfi");
}
