#ifndef CM_DIAGNOSTIC_USB_H
#define CM_DIAGNOSTIC_USB_H
#include "diagnostic.h"
enum { DG_USB_NONE, DG_USB_ACK, DG_USB_NAK, DG_USB_STALL };
typedef struct {
  uint8_t address, configured, halt_out, halt_in, toggle_out, toggle_in;
  uint8_t pending_address, pending_kind, resetting, out_overflow;
  uint8_t out_bytes[36], in_bytes[36];
  size_t out_used, in_length, in_position;
  uint32_t control_generation, pending_control, in_token;
  unsigned in_phase;
} DGUsb;
typedef struct {
  uint8_t bytes[32];
  size_t length;
  uint32_t token;
} DGControl;
/* Request-level control model: actual SETUP CRC and packetization are provider
 * duties. */
int dg_usb_setup(DGUsb *, const uint8_t[8], DGControl *);
int dg_usb_status(DGUsb *, uint32_t, unsigned);
int dg_usb_reset(DGUsb *, unsigned);
int dg_usb_out(DGUsb *, unsigned, const uint8_t *, size_t, unsigned,
               DGSession *, DGMessage *, unsigned *, int *);
int dg_usb_in_queue(DGUsb *, uint32_t, const DGMessage *);
int dg_usb_in_packet(const DGUsb *, uint32_t, const uint8_t **, size_t *,
                     unsigned *);
int dg_usb_in_ack(DGUsb *, uint32_t, unsigned);
int dg_usb_in_cancel(DGUsb *, uint32_t, unsigned);
#endif
