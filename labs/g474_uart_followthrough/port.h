#ifndef G474_UART_PORT_H
#define G474_UART_PORT_H
#include <stdint.h>
/* Original selected USART2/PA2, TX-only 8N1, FIFO-disabled HSI16 profile.
   Caller owns the peripheral, PA2 configuration and serialized RCC updates.
   No active transfer may exist at initialization; this is not hot reconfiguration. */
enum G4Reg {G4_RCC_CR,G4_RCC_AHB2ENR,G4_RCC_APB1ENR1,G4_RCC_APB1RSTR1,G4_RCC_CCIPR,
 G4_GPIO_MODER,G4_GPIO_OTYPER,G4_GPIO_OSPEEDR,G4_GPIO_PUPDR,G4_GPIO_AFRL,
 G4_USART_CR1,G4_USART_CR2,G4_USART_CR3,G4_USART_BRR,G4_USART_PRESC,G4_USART_ISR,
 G4_RCC_CFGR,G4_GPIO_AFRH,G4_REG_COUNT};
typedef uint32_t (*G4Read)(void *,enum G4Reg);
typedef void (*G4Write)(void *,enum G4Reg,uint32_t);
typedef struct {void *context;G4Read read;G4Write write;} G4Io;
enum {G4_OK=0,G4_INVALID=-1,G4_CLOCK_TIMEOUT=-2,G4_ENABLE_TIMEOUT=-3};
typedef struct {unsigned stage,polls;uint32_t brr;} G4Init;
/* On invalid admission, no effect and output unchanged. Failure records its
   stage and leaves any enabled shared clock intact; no fictitious rollback.
   poll_limit bounds reads per awaited flag, not elapsed time. */
int g4_initialize(G4Io,uint32_t baud,unsigned poll_limit,G4Init *);
G4Io g4_physical_io(void);
/* Supplied independent clock-observation adapter: requires exclusive PA8/MCO
   ownership, completed initialization and no previous MCO consumer. */
int g4_clock_probe(G4Io);
#endif
