#include "wire.h"
#include "com_net.h"
#ifndef NW_WIRE_MUTANT
#define NW_WIRE_MUTANT 0
#endif
static unsigned word(const unsigned char *p) {
  return (unsigned)p[0] * 256 + p[1];
}
static uint32_t dword(const unsigned char *p) {
  return ((uint32_t)word(p) << 16) | word(p + 2);
}
static int equal(const unsigned char *a, const unsigned char *b, unsigned n) {
  for (unsigned i = 0; i < n; ++i)
    if (a[i] != b[i])
      return 0;
  return 1;
}
int nw_ingress(NwRx *q, uint32_t *next, uint16_t port, const unsigned char *p,
               size_t n) {
  static const unsigned char mac[12] = {2, 0, 0, 0, 0, 2, 2, 0, 0, 0, 0, 1};
  static const unsigned char addresses[8] = {192, 0, 2, 1, 192, 0, 2, 2};
  if (!q || !next || !port || !p || n < 60 || n > 1514 || !equal(p, mac, 12) ||
      word(p + 12) != 0x800)
    return NW_INVALID;
  const unsigned char *ip = p + 14, *tcp = p + 34;
  unsigned total = word(ip + 2);
  if (ip[0] != 0x45 || total < 40 || total > 1500 || total > n - 14 ||
      (word(ip + 6) & 0xbfff) || !ip[8] || ip[9] != 6 ||
      !equal(ip + 12, addresses, 8) || net_checksum(ip, 20, 0) ||
      word(tcp) != port || word(tcp + 2) != 8080 || tcp[12] != 0x50 ||
      (tcp[13] != 0x10 && tcp[13] != 0x18))
    return NW_INVALID;
  uint32_t pseudo = word(addresses) + word(addresses + 2) +
                    word(addresses + 4) + word(addresses + 6) + 6 + total - 20;
  if (net_checksum(tcp, total - 20, pseudo))
    return NW_INVALID;
  if (dword(tcp + 4) != *next)
    return NW_STALE;
  size_t payload = NW_WIRE_MUTANT == 1 ? n - 54 : total - 40;
  int rc = nw_rx_offer(q, tcp + 20, payload);
  if (rc == NW_OK || NW_WIRE_MUTANT == 2)
    *next += (uint32_t)payload;
  return rc;
}
