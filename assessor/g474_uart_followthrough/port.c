#include "port.h"
#ifndef G4_MUTANT
#define G4_MUTANT 0
#endif
static void field(struct g4_io io,enum g4_reg reg,uint32_t mask,uint32_t value){io.write(io.context,reg,(io.read(io.context,reg)&~mask)|value);}
static int wait_flag(struct g4_io io,enum g4_reg reg,uint32_t mask,unsigned limit,struct g4_init *out){
 for(unsigned i=0;i<limit;i++){++out->polls;if(io.read(io.context,reg)&mask)return 1;}return 0;
}
int g4_initialize(struct g4_io io,uint32_t baud,unsigned limit,struct g4_init *out){
 if(!io.read || !io.write || !out || !baud || baud>1000000u || !limit || limit>1000000u)return G4_INVALID;
 uint32_t brr=(16000000u+baud/2u)/baud;if(brr<16 || brr>65535)return G4_INVALID;
 *out=(struct g4_init){1,0,brr};field(io,G4_RCC_CR,0,1u<<8);
 if(!wait_flag(io,G4_RCC_CR,1u<<10,limit,out))return G4_CLOCK_TIMEOUT;
 out->stage=2;field(io,G4_RCC_AHB2ENR,0,1);(void)io.read(io.context,G4_RCC_AHB2ENR);
 field(io,G4_RCC_APB1ENR1,0,1u<<17);(void)io.read(io.context,G4_RCC_APB1ENR1);
 /* Force/release reset only USART2, whose exclusive ownership is required. */
 field(io,G4_RCC_APB1RSTR1,0,1u<<17);field(io,G4_RCC_APB1RSTR1,1u<<17,0);
 field(io,G4_RCC_CCIPR,12u,G4_MUTANT==1?0:8u);
 out->stage=3;
 field(io,G4_GPIO_OTYPER,1u<<2,0);field(io,G4_GPIO_OSPEEDR,3u<<4,2u<<4);
 field(io,G4_GPIO_PUPDR,3u<<4,0);field(io,G4_GPIO_AFRL,15u<<8,(G4_MUTANT==2?8u:7u)<<8);
 field(io,G4_GPIO_MODER,3u<<4,2u<<4);
 /* UE=0, RE=0, FIFOEN=0, 8-bit/no parity/oversampling16; reset clears other modes. */
 io.write(io.context,G4_USART_CR1,0);io.write(io.context,G4_USART_CR2,0);io.write(io.context,G4_USART_CR3,0);
 io.write(io.context,G4_USART_PRESC,0);io.write(io.context,G4_USART_BRR,brr);
 out->stage=4;io.write(io.context,G4_USART_CR1,9u|(G4_MUTANT==3?1u<<29:0));
 if(G4_MUTANT!=4 && !wait_flag(io,G4_USART_ISR,1u<<21,limit,out)){io.write(io.context,G4_USART_CR1,0);return G4_ENABLE_TIMEOUT;}
 out->stage=5;return G4_OK;
}
