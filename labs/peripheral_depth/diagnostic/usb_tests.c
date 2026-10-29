#include "usb.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("DG_PUBLIC_USB_CHECK:%d\n", __LINE__);                            \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  struct dg_usb u = {0};
  struct dg_control c;
  uint8_t address[8] = {0, 5, 7, 0, 0, 0, 0, 0},
          configuration[8] = {0, 9, 1, 0, 0, 0, 0, 0};
  CHECK(dg_usb_setup(&u, address, &c) == DG_OK && !u.address);
  CHECK(dg_usb_status(&u, c.token, 1) == DG_OK && u.address == 7);
  CHECK(dg_usb_setup(&u, configuration, &c) == DG_OK && u.configured);
  struct dg_session s = {0};
  struct dg_message m = {0};
  unsigned h;
  int app;
  const uint8_t bytes[6] = {0, 1, 0, 2, 1, 2};
  CHECK(dg_usb_out(&u, 0, bytes, 6, 1, &s, &m, &h, &app) == DG_OK &&
        h == DG_USB_ACK && app == DG_OK);
  CHECK(m.sequence == 1 && m.length == 2 && m.payload[1] == 2);
  CHECK(dg_usb_out(&u, 0, bytes, 6, 1, &s, &m, &h, &app) == DG_OK &&
        app == DG_BUSY && s.accepted == 1);
  puts("DG_PUBLIC_USB_PASS");
  return 0;
}
