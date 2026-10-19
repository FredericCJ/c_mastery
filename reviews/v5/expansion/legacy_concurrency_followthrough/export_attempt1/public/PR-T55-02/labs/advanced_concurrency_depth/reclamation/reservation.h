#ifndef AR_RESERVATION_H
#define AR_RESERVATION_H
#include "reclamation.h"
#define AR_CAPACITY 4u
#define AR_LIMIT ((UINT_MAX >> 3) - 1u)
enum {
  AR_OK,
  AR_EMPTY,
  AR_PENDING,
  AR_FULL,
  AR_CANCELLED,
  AR_TOO_LATE,
  AR_INVALID,
  AR_EXHAUSTED,
  AR_UNIMPLEMENTED
};
typedef struct {
  atomic_uint state;
  APValue value;
} ARCell;
typedef struct ARQueue {
  atomic_uint head, tail;
  ARCell cells[AR_CAPACITY];
  APHook hook;
  void *context;
} ARQueue;
typedef struct {
  const ARQueue *owner;
  unsigned sequence;
} ARTicket;
void ar_init(ARQueue *, APHook, void *);
int ar_reserve(ARQueue *, ARTicket *);
/* Exactly one producer calls finish once; cancellation does not replace it. */
int ar_finish(ARQueue *, ARTicket, APValue);
int ar_cancel(ARQueue *, ARTicket);
int ar_pop(ARQueue *, APValue *);
#endif
