#ifndef B06_RECOVERY_TRANSPORT_H
#define B06_RECOVERY_TRANSPORT_H
#include "boot_storage.h"
enum { RT_CAPACITY=2156 };
enum rt_result { RT_BAD=-1,RT_MORE=0,RT_READY=1 };
struct rt_frame {unsigned used,need,state;unsigned char bytes[RT_CAPACITY];};
void rt_reset(struct rt_frame *);
enum rt_result rt_byte(struct rt_frame *,unsigned char);
/* Whole complete frame and current trusted policy. Output preserved on failure;
 * the owned input frame remains immutable throughout verification/writing. */
int rt_admit(const struct rt_frame *,const struct fb_policy *,const struct bs_layouts *,struct fb_image *);
/* Cold H745 clock profile already admitted, exclusive PD8/9 and USART3.
 * Return1 ready,0 budget exhausted,-1 invalid precondition. Init timeout disables
 * UE but retains clocks/pin config; not a rollback of all hardware state. */
int rt_uart_init(struct fh_io,unsigned budget);
/* Receive1/idle0/error-1; preserves byte on idle/error, returns observed status. */
int rt_uart_rx(struct fh_io,unsigned char *,uint32_t *);
/* Status byte transmission: budget separately bounds TXE and final TC polls. */
int rt_uart_tx(struct fh_io,unsigned char,unsigned budget);
/* No input1 / frame-ready2 / partial0 / hardware or format error-1.
 * Hardware/format error poisons the frame until explicit rt_reset. */
int rt_uart_step(struct fh_io,struct rt_frame *,uint32_t *);
#endif
