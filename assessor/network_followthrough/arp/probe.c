#include "arp.h"
#include <stdio.h>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x)) {                                                                \
      puts("NA_PROBE " #x);                                                    \
      return 1;                                                                \
    }                                                                          \
  } while (0)
int main(void) {
  const struct na_config config = {{2, 0, 0, 0, 0, 2}, {192, 0, 2, 2}, {192, 0, 2, 1}};
  const unsigned steps[] = {1, 17, 999, 1000, 2000, 4000};
  const uint64_t starts[] = {0, UINT32_MAX - 999, UINT64_MAX - 40000};
  for (unsigned b = 0; b < 3; ++b)
    for (unsigned k = 0; k < 6; ++k)
      for (unsigned mask = 0; mask < 4; ++mask) {
        struct na_arp state;
        struct net_packet packet;
        struct na_ticket ticket;
        unsigned byte = 17;
        CHECK(na_init(&state, config, 1) == NA_OK &&
              na_queue(&state, &byte, 1, starts[b]) == NA_OK);
        for (unsigned elapsed = 0, index = 0; elapsed <= 4000;
             elapsed += steps[k], ++index) {
          int rc = na_prepare(&state, starts[b] + elapsed, &packet, &ticket);
          unsigned accepted = ((index + mask) % 4) != 0;
          if (rc == NA_OK) {
            CHECK(ticket.kind == NA_REQUEST);
            CHECK(na_submit(&state, ticket, accepted) == NA_OK);
          }
          printf("TIME %u %u %u %u %u %d %u %u %u\n", b, steps[k], mask, index,
                 elapsed, rc, accepted, state.requests, state.pending);
        }
      }
  return 0;
}
