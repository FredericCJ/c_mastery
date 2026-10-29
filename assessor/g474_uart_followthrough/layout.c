#include <stddef.h>
#include "stm32g474xx.h"
/* Compile-only comparison with primary vendor definitions. The vendor object
   is never linked into the strict image. Deliberate old-map negative. */
#ifndef G4_OLD_MAP
#define G4_OLD_MAP 0
#endif
_Static_assert(offsetof(USART_TypeDef, ISR) == (G4_OLD_MAP ? 0 : 0x1c),
               "status mapping");
_Static_assert(offsetof(USART_TypeDef, TDR) == 0x28 &&
                   offsetof(USART_TypeDef, PRESC) == 0x2c,
               "data/prescaler mapping");
_Static_assert(offsetof(RCC_TypeDef, APB1RSTR1) == 0x38 &&
                   offsetof(RCC_TypeDef, AHB2ENR) == 0x4c &&
                   offsetof(RCC_TypeDef, APB1ENR1) == 0x58 &&
                   offsetof(RCC_TypeDef, CCIPR) == 0x88,
               "RCC offsets");
_Static_assert(RCC_BASE == 0x40021000 && GPIOA_BASE == 0x48000000 &&
                   USART2_BASE == 0x40004400,
               "bases");
_Static_assert(DBGMCU_BASE == 0xe0042000, "debug identity only");
_Static_assert(offsetof(RCC_TypeDef, CFGR) == 8 &&
                   offsetof(GPIO_TypeDef, AFR[1]) == 0x24,
               "MCO and AFH offsets");
_Static_assert(RCC_CFGR_MCOSEL == 0x0f000000 && RCC_CFGR_MCOPRE == 0x70000000 &&
                   RCC_CFGR_MCOPRE_DIV16 == 0x40000000,
               "MCO fields");
_Static_assert(RCC_CR_HSION == (1u << 8) && RCC_CR_HSIRDY == (1u << 10),
               "HSI flags");
_Static_assert(RCC_APB1ENR1_USART2EN == (1u << 17) &&
                   RCC_APB1RSTR1_USART2RST == (1u << 17),
               "clock/reset");
_Static_assert(RCC_CCIPR_USART2SEL == 12 && RCC_CCIPR_USART2SEL_1 == 8,
               "kernel selector");
_Static_assert(USART_CR1_UE == 1 && USART_CR1_TE == 8 &&
                   USART_CR1_FIFOEN == (1u << 29) &&
                   USART_ISR_TEACK == (1u << 21) && USART_ICR_TCCF == 64,
               "mode and acknowledge");
