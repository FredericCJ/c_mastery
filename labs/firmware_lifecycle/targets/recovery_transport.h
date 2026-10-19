#ifndef B06_RECOVERY_TRANSPORT_H
#define B06_RECOVERY_TRANSPORT_H
#include "boot_storage.h"
enum { RT_CAPACITY=2156 };
enum RTResult { RT_BAD=-1,RT_MORE=0,RT_READY=1 };
typedef struct {unsigned used,need,state;unsigned char bytes[RT_CAPACITY];} RTFrame;
void rt_reset(RTFrame *);
enum RTResult rt_byte(RTFrame *,unsigned char);
/* Whole complete frame and current trusted policy. Output preserved on failure;
 * the owned input frame remains immutable throughout verification/writing. */
int rt_admit(const RTFrame *,const FBPolicy *,const BSLayouts *,FBImage *);
/* Cold H745 clock profile already admitted, exclusive PD8/9 and USART3.
 * Return1 ready,0 budget exhausted,-1 invalid precondition. Init timeout disables
 * UE but retains clocks/pin config; not a rollback of all hardware state. */
int rt_uart_init(FHIO,unsigned budget);
/* Receive1/idle0/error-1; preserves byte on idle/error, returns observed status. */
int rt_uart_rx(FHIO,unsigned char *,uint32_t *);
/* Status byte transmission: budget separately bounds TXE and final TC polls. */
int rt_uart_tx(FHIO,unsigned char,unsigned budget);
/* No input1 / frame-ready2 / partial0 / hardware or format error-1.
 * Hardware/format error poisons the frame until explicit rt_reset. */
int rt_uart_step(FHIO,RTFrame *,uint32_t *);
#endif
