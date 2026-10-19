#define _POSIX_C_SOURCE 200809L
#include "diagnostic.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/usbdevice_fs.h>
#include <poll.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      printf("DG_GUEST_CHECK:%d errno=%d\n", __LINE__, errno);                 \
      return 1;                                                                \
    }                                                                          \
  } while (0)
static int number(const char *directory, const char *name, int hex) {
  char path[512];
  snprintf(path, sizeof path, "%s/%s", directory, name);
  FILE *f = fopen(path, "r");
  unsigned v = 0;
  int result = -1;
  if (f) {
    if (fscanf(f, hex ? "%x" : "%u", &v) == 1 && v <= INT_MAX)
      result = (int)v;
    fclose(f);
  }
  return result;
}
static int open_target(void) {
  for (unsigned wait = 0; wait < 100; ++wait) {
    DIR *d = opendir("/sys/bus/usb/devices");
    struct dirent *e;
    if (d) {
      while ((e = readdir(d))) {
        char path[512];
        snprintf(path, sizeof path, "/sys/bus/usb/devices/%s", e->d_name);
        if (number(path, "idVendor", 1) == 65535 &&
            number(path, "idProduct", 1) == 1) {
          int bus = number(path, "busnum", 0),
              device = number(path, "devnum", 0);
          if (bus > 0 && device > 0) {
            snprintf(path, sizeof path, "/dev/bus/usb/%03d/%03d", bus, device);
            int fd = open(path, O_RDWR);
            if (fd >= 0) {
              closedir(d);
              return fd;
            }
          }
        }
      }
      closedir(d);
    }
    struct timespec delay = {0, 100000000};
    nanosleep(&delay, 0);
  }
  return -1;
}
static int control(int fd, unsigned type, unsigned request, unsigned value,
                   unsigned index, void *bytes, unsigned length) {
  struct usbdevfs_ctrltransfer q = {.bRequestType = (unsigned char)type,
                                    .bRequest = (unsigned char)request,
                                    .wValue = (unsigned short)value,
                                    .wIndex = (unsigned short)index,
                                    .wLength = (unsigned short)length,
                                    .timeout = 1000,
                                    .data = bytes};
  return ioctl(fd, USBDEVFS_CONTROL, &q);
}
static int bulk(int fd, unsigned ep, void *data, unsigned length) {
  struct usbdevfs_bulktransfer r = {
      .ep = ep, .len = length, .timeout = 1000, .data = data};
  return ioctl(fd, USBDEVFS_BULK, &r);
}
static unsigned word(const unsigned char *p) {
  return (unsigned)p[0] + ((unsigned)p[1] << 8) + ((unsigned)p[2] << 16) +
         ((unsigned)p[3] << 24);
}
int main(void) {
  int fd = open_target();
  CHECK(fd >= 0);
  unsigned char config[32], snapshot[24];
  CHECK(control(fd, 128, 6, 512, 0, config, sizeof config) == 32 &&
        config[2] == 32 && config[4] == 1);
  unsigned epout = config[20], epin = config[27];
  CHECK((epin & 128) && !(epout & 128));
  unsigned interface = 0;
  CHECK(ioctl(fd, USBDEVFS_CLAIMINTERFACE, &interface) == 0);
  unsigned char wire[40], echo[40];
  size_t n;
  for (unsigned length = 0; length <= 32; ++length) {
    DGMessage m = {(uint16_t)(length + 1), (uint16_t)length, {0}};
    for (unsigned i = 0; i < length; ++i)
      m.payload[i] = (uint8_t)(i ^ length);
    CHECK(dg_encode(&m, wire, sizeof wire, &n) == DG_OK);
    CHECK(bulk(fd, epout, wire, (unsigned)n) == (int)n);
    if (n % 8 == 0)
      CHECK(bulk(fd, epout, wire, 0) == 0);
    CHECK(bulk(fd, epin, echo, sizeof echo) == (int)n &&
          memcmp(wire, echo, n) == 0);
  }
  CHECK(control(fd, 0xc0, 0x51, 0, 0, snapshot, sizeof snapshot) == 24);
  printf("DG_GUEST_COUNTERS %u %u %u %u %u %u\n", word(snapshot),
         word(snapshot + 4), word(snapshot + 8), word(snapshot + 12),
         word(snapshot + 16), word(snapshot + 20));
  CHECK(word(snapshot) == 33 && word(snapshot + 4) == 0 &&
        word(snapshot + 8) >= 1 && word(snapshot + 12) == 33);
  printf("DG_GUEST_ECHO accepted=%u inline=%u completed=%u\n", word(snapshot),
         word(snapshot + 8), word(snapshot + 12));
  /* A valid prefix of an overlong transfer must never dispatch. */
  DGMessage m = {34, 32, {0}};
  CHECK(dg_encode(&m, wire, sizeof wire, &n) == DG_OK);
  wire[36] = 0xa5;
  CHECK(bulk(fd, epout, wire, 37) == 37);
  CHECK(control(fd, 0xc0, 0x51, 0, 0, snapshot, sizeof snapshot) == 24 &&
        word(snapshot) == 33 && word(snapshot + 4) == 1);
  CHECK(bulk(fd, epout, wire, 36) == 36);
  CHECK(bulk(fd, epin, echo, sizeof echo) == 36);
  CHECK(bulk(fd, epout, wire, 36) == 36);
  CHECK(control(fd, 0xc0, 0x51, 0, 0, snapshot, sizeof snapshot) == 24);
  printf("DG_GUEST_REPLAY accepted=%u rejected=%u\n", word(snapshot),
         word(snapshot + 4));
  CHECK(word(snapshot) == 34 && word(snapshot + 4) == 2);
  CHECK(control(fd, 0x40, 0x52, 0, 0, 0, 0) == 0);
  CHECK(control(fd, 0xc0, 0x51, 0, 0, snapshot, sizeof snapshot) == 24 &&
        word(snapshot + 20) >= 1);
  CHECK(control(fd, 0x40, 0x53, 0, 0, 0, 0) == 0);
  printf("DG_GUEST_DEQUEUE callbacks=%u\n", word(snapshot + 20));
  struct usbdevfs_urb urb = {.type = USBDEVFS_URB_TYPE_BULK,
                             .endpoint = (unsigned char)epin,
                             .buffer = echo,
                             .buffer_length = sizeof echo};
  CHECK(ioctl(fd, USBDEVFS_SUBMITURB, &urb) == 0);
  CHECK(ioctl(fd, USBDEVFS_DISCARDURB, &urb) == 0);
  struct pollfd p = {fd, POLLOUT, 0};
  CHECK(poll(&p, 1, 2000) == 1);
  void *returned = 0;
  CHECK(ioctl(fd, USBDEVFS_REAPURBNDELAY, &returned) == 0 && returned == &urb &&
        urb.status < 0 && urb.actual_length == 0);
  printf("DG_GUEST_HOST_CANCEL status=%d bytes=%d reaped=1\n", urb.status,
         urb.actual_length);
  CHECK(ioctl(fd, USBDEVFS_RELEASEINTERFACE, &interface) == 0);
  CHECK(close(fd) == 0);
  puts("DG_GUEST_PASS");
  return 0;
}
