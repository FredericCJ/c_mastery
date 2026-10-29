/* Reuse the prior original UART effect model and actual reference driver. */
#define main prior_uart_tests
#include "../../../labs/peripheral_depth/uart/tests.c"
#undef main
#include "uart_adapter.h"

int main(void)
{
    for (unsigned backend = 0; backend <= 2; backend++)
        for (unsigned cut = 0; cut <= 8; cut++)
        {
            struct dg_device device = {0};
            device.source_after = 1;
            struct pu_io io = {&device, rd, wr};
            struct pu_clock clock = {16000000, 1, 1, 1, 1, 0};
            struct pl_uart uart = {0};
            CHECK(pl_uart_open(&uart, io, &clock, 115200, backend, 2) == PL_OK);
            struct pl_service service = {0};
            struct pl_transport t = pl_uart_transport(&uart);
            struct pl_result result = {99, 99, 99};
            struct pl_parser peer = {0};
            struct pl_command command = {99, 99};
            device.sr = PU_TXE | PU_TC;
            CHECK(pl_send(&service, t, 0xf0, 0xf1, 1, 0, 100) == PL_OK);
            for (unsigned k = 0; k < cut; k++)
            {
                if (backend == PU_DMA)
                {
                    const uint8_t *p = (const uint8_t *)device.address;
                    device.wire[device.writes++] = p[k];
                    device.dma_count--;
                }
                else
                    CHECK(pl_poll(&service, t, 1, k + 1, &result) == PL_BUSY &&
                          result.token == 99);
            }
            unsigned committed = 0;
            for (unsigned k = 0; k < device.writes; k++)
            {
                int r = pl_feed(&peer, device.wire[k], &command);
                committed += r == PL_OK;
                CHECK(r == PL_BUSY || r == PL_OK);
            }
            CHECK(committed == (cut == 8));
            CHECK(pl_cancel(&service, t, 1) == PL_OK && service.active);
            if (backend == PU_DMA)
            {
                device.sr |= PU_TC;
                CHECK(pl_poll(&service, t, 1, 10, &result) == PL_BUSY &&
                      uart.driver.memory_owned && result.token == 99);
                device.dma_busy = 0;
                device.dma_flags = 1;
            }
            else if (cut)
                CHECK(pl_poll(&service, t, 1, 10, &result) == PL_BUSY &&
                      uart.driver.line_owned);
            device.sr |= PU_TC;
            CHECK(pl_poll(&service, t, 1, 11, &result) == PL_OK &&
                  result.status == PL_CANCELLED &&
                  result.may_have_committed == 1 && !uart.driver.active &&
                  !service.active);
            device.writes = 0;
            device.dma_count = 0;
            device.dma_flags = 0;
            device.sr = PU_TXE | PU_TC;
            CHECK(pl_send(&service, t, 7, 9, 2, 12, 100) == PL_OK);
            for (unsigned k = 0; k < 8; k++)
            {
                if (backend == PU_DMA)
                {
                    const uint8_t *p = (const uint8_t *)device.address;
                    device.wire[device.writes++] = p[k];
                    device.dma_count--;
                }
                else
                    CHECK(pl_poll(&service, t, 2, 13 + k, &result) == PL_BUSY);
            }
            if (backend == PU_DMA)
            {
                device.dma_busy = 0;
                device.dma_flags = 1;
            }
            device.sr |= PU_TC;
            CHECK(pl_poll(&service, t, 2, 21, &result) == PL_OK &&
                  result.status == PL_OK);
            unsigned new_commits = 0;
            for (unsigned k = 0; k < 8; k++)
            {
                int r = pl_feed(&peer, device.wire[k], &command);
                new_commits += r == PL_OK;
                CHECK(r == PL_BUSY || r == PL_OK);
            }
            CHECK(new_commits == 1 && command.sequence == 7 &&
                  command.value == 9 && !device.bad);
        }
    puts("PL_UART_PASS");
    return 0;
}
