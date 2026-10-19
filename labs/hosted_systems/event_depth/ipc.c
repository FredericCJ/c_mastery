#include "ipc.h"
int ei_init(EiRequest *r, const unsigned char p[8], unsigned epoch,
            unsigned id) {
  (void)r;
  (void)p;
  (void)epoch;
  (void)id;
  return -1;
}
int ei_transition(EiRequest *r, enum EiEvent e) {
  (void)r;
  (void)e;
  return -1;
}
int ei_invariant(const EiRequest *r) {
  (void)r;
  return 0;
}
int ei_enqueue(EiQueue *q, EiRequest *r) {
  (void)q;
  (void)r;
  return -1;
}
int ei_dequeue(EiQueue *q, EiRequest **r) {
  (void)q;
  (void)r;
  return -1;
}
void ei_reply(const unsigned char in[8], unsigned char out[4]) {
  (void)in;
  (void)out;
}
