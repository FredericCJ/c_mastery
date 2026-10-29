#include "usb.h"
#ifndef DG_USB_MUTANT
#define DG_USB_MUTANT 0
#endif
static uint16_t le16(const uint8_t *p) {
  return (uint16_t)(p[0] + (uint16_t)p[1] * 256u);
}
static const uint8_t device[18] = {18,  1, 0, 2, 0, 0, 0, 8, 255,
                                   255, 1, 0, 0, 1, 0, 0, 0, 1};
static const uint8_t configuration[32] = {
    9, 2, 32, 0, 1, 1, 0, 128, 50, 9, 4, 0,   0, 2, 255, 0,
    0, 0, 7,  5, 1, 2, 8, 0,   0,  7, 5, 129, 2, 8, 0,   0};
static void endpoints(struct dg_usb *u) {
  u->halt_out = u->halt_in = u->toggle_out = u->toggle_in = u->out_overflow = 0;
  u->out_used = 0;
}
int dg_usb_setup(struct dg_usb *u, const uint8_t q[8], struct dg_control *out) {
  if (!u || !q || !out)
    return DG_INVALID;
  /* Arrival of any new SETUP cancels the earlier control status callback. */
  u->pending_control = 0;
  u->pending_kind = 0;
  if (u->resetting)
    return DG_BUSY;
  if (u->control_generation == UINT32_MAX)
    return DG_FULL;
  unsigned type = q[0], request = q[1];
  uint16_t value = le16(q + 2), index = le16(q + 4), length = le16(q + 6);
  struct dg_control r = {{0}, 0, 0};
  unsigned kind = 0;
  const uint8_t *descriptor = 0;
  size_t count = 0;
  if (type == 128 && request == 6 && !index && !(value & 255u)) {
    if (value == 256) {
      descriptor = device;
      count = sizeof device;
    } else if (value == 512) {
      descriptor = configuration;
      count = sizeof configuration;
    } else
      return DG_FRAME; /* Full-speed-only device: qualifier also stalls. */
    if (count > length)
      count = length;
    for (size_t i = 0; i < count; ++i)
      r.bytes[i] = descriptor[i];
    r.length = count;
  } else if (type == 0 && request == 5 && value > 0 && value <= 127 && !index &&
             !length && !u->configured) {
    kind = 1;
    u->pending_address = (uint8_t)value;
    if (DG_USB_MUTANT == 1)
      u->address = (uint8_t)value;
  } else if (type == 0 && request == 9 && value <= 1 && !index && !length &&
             u->address) {
    if (u->in_phase)
      return DG_BUSY; /* Provider must quiesce pending requests first. */
    u->configured = (uint8_t)value;
    endpoints(u);
  } else if (type == 128 && request == 8 && !value && !index && length == 1 &&
             u->address) {
    r.bytes[0] = u->configured;
    r.length = 1;
  } else if (type == 129 && request == 10 && !value && !index && length == 1 &&
             u->configured) {
    r.length = 1;
  } else if (type == 1 && request == 11 && !value && !index && !length &&
             u->configured) {
    if (u->in_phase)
      return DG_BUSY;
    endpoints(u);
  } else if (type == 2 && (request == 1 || request == 3) && !value && !length &&
             (index == 1 || index == 129) && u->configured) {
    if (index == 129 && u->in_phase)
      return DG_BUSY;
    uint8_t *halt = index == 1 ? &u->halt_out : &u->halt_in;
    uint8_t *toggle = index == 1 ? &u->toggle_out : &u->toggle_in;
    *halt = (uint8_t)(request == 3);
    if (request == 1) {
      if (DG_USB_MUTANT != 2)
        *toggle = 0;
      if (index == 1) {
        u->out_used = 0;
        u->out_overflow = 0;
      }
    }
  } else if (request == 0 && !value && length == 2 &&
             ((type == 128 && !index) ||
              (type == 129 && !index && u->configured) ||
              (type == 130 && (index == 1 || index == 129) && u->configured))) {
    r.length = 2;
    if (type == 130)
      r.bytes[0] = index == 1 ? u->halt_out : u->halt_in;
  } else
    return DG_FRAME;
  r.token = ++u->control_generation;
  u->pending_control = r.token;
  u->pending_kind = (uint8_t)kind;
  *out = r;
  return DG_OK;
}
int dg_usb_status(struct dg_usb *u, uint32_t token, unsigned acknowledged) {
  if (!u || acknowledged > 1)
    return DG_INVALID;
  if (!token || token != u->pending_control)
    return DG_STALE;
  if (acknowledged && u->pending_kind == 1)
    u->address = u->pending_address;
  u->pending_control = 0;
  u->pending_kind = 0;
  return DG_OK;
}
int dg_usb_reset(struct dg_usb *u, unsigned quiesced) {
  if (!u || quiesced > 1)
    return DG_INVALID;
  u->address = u->configured = 0;
  u->pending_control = 0;
  u->pending_kind = 0;
  endpoints(u);
  if (u->in_phase && !quiesced) {
    u->resetting = 1;
    u->in_phase = 2;
    return DG_BUSY;
  }
  u->in_phase = 0;
  u->in_length = u->in_position = 0;
  u->resetting = 0;
  return DG_OK;
}
int dg_usb_out(struct dg_usb *u, unsigned toggle, const uint8_t *p, size_t n,
               unsigned crc, struct dg_session *s, struct dg_message *m, unsigned *handshake,
               int *application) {
  if (!u || !s || !m || !handshake || !application || toggle > 1 || crc > 1 ||
      n > 8 || (n && !p))
    return DG_INVALID;
  *handshake = DG_USB_NONE;
  *application = DG_BUSY;
  if (!crc)
    return DG_OK;
  if (!u->configured || u->resetting) {
    *handshake = DG_USB_NAK;
    return DG_OK;
  }
  if (u->halt_out) {
    *handshake = DG_USB_STALL;
    return DG_OK;
  }
  *handshake = DG_USB_ACK;
  if (DG_USB_MUTANT != 3 && toggle != u->toggle_out)
    return DG_OK;
  u->toggle_out ^= 1;
  if (n > 36 - u->out_used)
    u->out_overflow = 1;
  if (!u->out_overflow) {
    for (size_t i = 0; i < n; ++i)
      u->out_bytes[u->out_used + i] = p[i];
    u->out_used += n;
  }
  if (n < 8) {
    *application =
        u->out_overflow ? DG_FRAME : dg_accept(s, u->out_bytes, u->out_used, m);
    u->out_used = 0;
    u->out_overflow = 0;
  }
  return DG_OK;
}
int dg_usb_in_queue(struct dg_usb *u, uint32_t token, const struct dg_message *m) {
  if (!u || !token)
    return DG_INVALID;
  if (!u->configured || u->resetting || u->in_phase || u->halt_in)
    return DG_BUSY;
  if (token <= u->in_token)
    return DG_STALE;
  size_t n;
  int rc = dg_encode(m, u->in_bytes, sizeof u->in_bytes, &n);
  if (rc)
    return rc;
  u->in_token = token;
  u->in_length = n;
  u->in_position = 0;
  u->in_phase = 1;
  return DG_OK;
}
int dg_usb_in_packet(const struct dg_usb *u, uint32_t token, const uint8_t **p,
                     size_t *n, unsigned *toggle) {
  if (!u || !p || !n || !toggle)
    return DG_INVALID;
  if (!token || token != u->in_token || !u->in_phase)
    return DG_STALE;
  if (u->in_phase != 1 || u->resetting || u->halt_in)
    return DG_BUSY;
  size_t remaining = u->in_length - u->in_position;
  *n = remaining > 8 ? 8 : remaining;
  *p = u->in_bytes + u->in_position;
  *toggle = u->toggle_in;
  return DG_OK;
}
int dg_usb_in_ack(struct dg_usb *u, uint32_t token, unsigned ack) {
  if (!u || ack > 1)
    return DG_INVALID;
  const uint8_t *p;
  size_t n;
  unsigned toggle;
  int rc = dg_usb_in_packet(u, token, &p, &n, &toggle);
  if (rc)
    return rc;
  if (!ack)
    return DG_OK;
  u->in_position += n;
  u->toggle_in ^= 1;
  if (n < 8 || (DG_USB_MUTANT == 4 && u->in_position == u->in_length))
    u->in_phase = 0;
  return DG_OK;
}
int dg_usb_in_cancel(struct dg_usb *u, uint32_t token, unsigned quiesced) {
  if (!u || quiesced > 1)
    return DG_INVALID;
  if (!token || token != u->in_token || !u->in_phase)
    return DG_STALE;
  if (!quiesced && DG_USB_MUTANT != 5) {
    u->in_phase = 2;
    return DG_BUSY;
  }
  u->in_phase = 0;
  return DG_OK;
}
