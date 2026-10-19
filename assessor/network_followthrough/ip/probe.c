#include "specimens.h"
#include <stdio.h>
#include <string.h>
int main(void) {
  NiReassembly s;
  NiPacket output;
  unsigned char packet[1500];
  char line[200], action;
  unsigned slots, policy, id, protocol, offset, length, more, pattern, index;
  unsigned long long now, timeout;
  int rc;
  while (fgets(line, sizeof line, stdin)) {
    index = 99;
    action = line[0];
    if (action == 'I') {
      if (sscanf(line, "I %u %u %llu %llu", &slots, &policy, &now, &timeout) !=
          4)
        return 2;
      rc = ni_init(&s, (NiConfig){0xc0000202u, 0xc0000201u}, slots, policy,
                   (uint64_t)now, (uint64_t)timeout);
    } else if (action == 'F') {
      if (sscanf(line, "F %u %u %u %u %u %llu %u", &id, &protocol, &offset,
                 &length, &more, &now, &pattern) != 7 ||
          length > 1480 || offset % 8)
        return 2;
      ni_test_header(packet, length, id, offset, more, protocol);
      for (unsigned i = 0; i < length; i++)
        packet[20 + i] = (unsigned char)(offset + i + pattern);
      rc = ni_receive(&s, packet, 20 + length, (uint64_t)now, &index);
    } else if (action == 'T') {
      if (sscanf(line, "T %llu", &now) != 1)
        return 2;
      rc = ni_tick(&s, (uint64_t)now);
    } else if (action == 'D') {
      if (sscanf(line, "D %u", &index) != 1)
        return 2;
      rc = ni_take(&s, index, &output);
    } else
      return 2;
    printf("%c %d %u", action, rc, index);
    for (unsigned i = 0; i < s.slots; i++) {
      NiSlot *a = &s.slot[i];
      printf(" %u", a->state);
      if (a->state)
        printf(":%u:%u:%llu", a->covered, a->total,
               (unsigned long long)a->expiry);
    }
    if (action == 'D' && rc == NI_OK) {
      printf(" bytes=");
      for (size_t i = 0; i < output.length; i++)
        printf("%02x", output.bytes[i]);
    }
    puts("");
  }
  return ferror(stdin) ? 2 : 0;
}
