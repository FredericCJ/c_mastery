#include "compare.h"
#include "registers.h"
#ifndef PL_NATIVE
#ifdef PL_G474
#include "stm32g4xx_hal.h"
#define STATUS_FIELD ISR
#define DATA_FIELD TDR
#define TC_FLAG USART_ISR_TC
#else
#include "stm32f4xx_hal.h"
#define STATUS_FIELD SR
#define DATA_FIELD DR
#define TC_FLAG USART_SR_TC
#endif
#endif
#include <stddef.h>
#ifndef PL_GUEST
#include <stdio.h>
#define REPORT(x) puts(x)
#else
void pl_report(const char *);
#define REPORT(x) pl_report(x)
#endif
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      REPORT("PL_COMPARE_CHECK:" #x);                                          \
      return 1;                                                                \
    }                                                                          \
  } while (0)
#ifndef PL_NATIVE
#ifdef PL_G474
_Static_assert(offsetof(USART_TypeDef, ISR) == PL_G474_STATUS &&
                   offsetof(USART_TypeDef, TDR) == PL_G474_DATA,
               "G474 map");
_Static_assert(USART_ISR_TXE_TXFNF == PL_G474_TXE && USART_ISR_TC == PL_G474_TC,
               "G474 flags");
#else
_Static_assert(offsetof(USART_TypeDef, SR) == PL_F439_STATUS &&
                   offsetof(USART_TypeDef, DR) == PL_F439_DATA,
               "inspected F439 map");
_Static_assert(USART_SR_TXE == PL_F439_TXE && USART_SR_TC == PL_F439_TC,
               "inspected flags");
#endif
#endif
typedef struct {
  volatile uint32_t *status, *data;
  uint32_t clock, ready, complete, calls, frozen;
} Model;
static Model *current;
uint32_t HAL_GetTick(void) {
  Model *m = current;
  m->calls++;
  if (m->calls >= m->ready)
    *m->status |= 128;
  if (m->calls >= m->complete && *m->data != 999)
    *m->status |= 64;
  return m->frozen ? m->clock : m->clock++;
}
static uint32_t tick(void *p) {
  current = p;
  return HAL_GetTick();
}
int main(void) {
#ifdef PL_G474
  for (unsigned revision = 1; revision < 2; revision++)
#elif defined(PL_NATIVE)
  for (unsigned revision = 0; revision < 2; revision++)
#else
  for (unsigned revision = 0; revision < 1; revision++)
#endif
    for (unsigned ready = 1; ready <= 4; ready++)
      for (unsigned complete = ready + 1; complete <= 7; complete++)
        for (unsigned budget = 0; budget < 9; budget++)
          for (unsigned wrap = 0; wrap < 2; wrap++) {
            int first = -1;
            uint32_t first_data = 0, first_calls = 0;
            for (unsigned mode = 0; mode <
#ifdef PL_NATIVE
                                    2u
#else
                                    3u
#endif
                 ;
                 mode++) {
              volatile uint32_t words[12] = {0};
              unsigned so = revision ? 7 : 0, d = revision ? 10 : 1;
              words[d] = 999;
              Model m = {words + so, words + d, wrap ? UINT32_MAX - 2 : 0,
                         ready,      complete,  0,
                         0};
              current = &m;
              int r;
#ifndef PL_NATIVE
              if (mode == 2) {
                /* Real vendor code over a RAM register object; flags/ticks
                 * supplied by this model. */
                USART_TypeDef device = {0};
                device.DATA_FIELD = 999;
                m.status = &device.STATUS_FIELD;
                m.data = &device.DATA_FIELD;
                UART_HandleTypeDef uart = {0};
                uart.Instance = &device;
                uart.Init.WordLength = UART_WORDLENGTH_8B;
                uart.Init.Parity = UART_PARITY_NONE;
                uart.gState = HAL_UART_STATE_READY;
                HAL_StatusTypeDef hr =
                    HAL_UART_Transmit(&uart, (const uint8_t[]){42}, 1, budget);
                r = hr == HAL_OK        ? PL_OK
                    : hr == HAL_TIMEOUT ? PL_TIMEOUT
                                        : PL_IO;
                CHECK(uart.gState == HAL_UART_STATE_READY);
                words[d] = device.DATA_FIELD;
                words[so] = device.STATUS_FIELD;
              } else
#endif
              {
                PlBytePort p =
                    mode == 1 ? (revision ? pl_direct_g474(words, tick, &m)
                                          : pl_direct_f439(words, tick, &m))
                              : pl_generated(words, revision, tick, &m);
                r = pl_byte(p, 42, budget);
              }
              /* Independent event reasoning: progress clock counts each failed
               * flag observation; readiness can appear at a timeout-check and
               * timeout still wins that check. */
              unsigned expected_calls = 1, sent = 0;
              int expected = PL_OK;
              while (expected_calls < ready) {
                expected_calls++;
                if (expected_calls - 1 > budget || !budget) {
                  expected = PL_TIMEOUT;
                  break;
                }
              }
              if (expected == PL_OK) {
                sent = 1;
                while (expected_calls < complete) {
                  expected_calls++;
                  if (expected_calls - 1 > budget || !budget) {
                    expected = PL_TIMEOUT;
                    break;
                  }
                }
              }
              CHECK(r == expected && words[d] == (sent ? 42u : 999u) &&
                    m.calls == expected_calls);
              if (mode == 0) {
                first = r;
                first_data = words[d];
                first_calls = m.calls;
              } else
                CHECK(r == first && words[d] == first_data &&
                      m.calls == first_calls);
              if (revision)
                CHECK(words[0] == 0 &&
                      words[1] ==
                          0); /* stale F4 offsets would corrupt CR1/CR2 */
            }
          }
#ifndef PL_NATIVE
  /* HAL READY after timeout is not proof of wire completion. */
  {
    USART_TypeDef device = {0};
    device.DATA_FIELD = 999;
    Model m = {&device.STATUS_FIELD, &device.DATA_FIELD, 0, 1, 99, 0, 0};
    current = &m;
    UART_HandleTypeDef uart = {0};
    uart.Instance = &device;
    uart.gState = HAL_UART_STATE_READY;
    CHECK(HAL_UART_Transmit(&uart, (const uint8_t[]){7}, 1, 1) == HAL_TIMEOUT);
    CHECK(device.DATA_FIELD == 7 && !(device.STATUS_FIELD & TC_FLAG) &&
          uart.gState == HAL_UART_STATE_READY);
    /* Wrapper must retain the line lease; this is a counterexample to
     * READY=>quiescent. */
  }
#endif
  REPORT("PL_COMPARE_PASS");
  return 0;
}
