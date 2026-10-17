#ifndef B03_DEPTH_H
#define B03_DEPTH_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
enum B3Status { B3_OK, B3_INVALID, B3_WAIT, B3_YIELD, B3_FAILED, B3_STALE, B3_AMBIGUOUS };
typedef long (*B3Write)(void *, const unsigned char *, size_t);
typedef struct { const unsigned char *bytes; size_t length,position; int terminal; } B3Writer;
int b3_pump(B3Writer *, B3Write, void *, unsigned budget);
enum B3Phase { B3_IDLE, B3_ACTIVE, B3_CANCELLING, B3_COMPLETED, B3_CANCELLED };
enum B3Event { B3_TIMEOUT, B3_COMPLETE, B3_CANCEL_ACK };
typedef struct { unsigned generation,phase,owned,reports; } B3Operation;
int b3_start(B3Operation *);
int b3_event(B3Operation *, unsigned generation, enum B3Event);
typedef struct { unsigned status,data,reads,writes; } B3Device;
int b3_ack(B3Device *, unsigned mask);
int b3_read_data(B3Device *, unsigned *out);
int b3_due(uint8_t now,uint8_t start,uint8_t delay,int *out);
typedef struct { atomic_uint ready; int payload; } B3Mailbox;
void b3_mailbox_init(B3Mailbox *);
int b3_publish(B3Mailbox *,int value);
int b3_take(B3Mailbox *,int *out);
/* These order choices are used by both handoffs, allowing source-bound review. */
memory_order b3_load_order(void);
memory_order b3_store_order(void);
#endif
