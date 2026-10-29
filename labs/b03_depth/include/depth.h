#ifndef B03_DEPTH_H
#define B03_DEPTH_H
#include <stddef.h>
#include <stdint.h>
#include <stdatomic.h>
enum b3_status { B3_OK, B3_INVALID, B3_WAIT, B3_YIELD, B3_FAILED, B3_STALE, B3_AMBIGUOUS };
typedef long (*b3_write_fn)(void *, const unsigned char *, size_t);
struct b3_writer { const unsigned char *bytes; size_t length,position; int terminal; };
int b3_pump(struct b3_writer *, b3_write_fn, void *, unsigned budget);
enum b3_phase { B3_IDLE, B3_ACTIVE, B3_CANCELLING, B3_COMPLETED, B3_CANCELLED };
enum b3_event { B3_TIMEOUT, B3_COMPLETE, B3_CANCEL_ACK };
struct b3_operation { unsigned generation,phase,owned,reports; };
int b3_start(struct b3_operation *);
int b3_event(struct b3_operation *, unsigned generation, enum b3_event);
struct b3_device { unsigned status,data,reads,writes; };
int b3_ack(struct b3_device *, unsigned mask);
int b3_read_data(struct b3_device *, unsigned *out);
int b3_due(uint8_t now,uint8_t start,uint8_t delay,int *out);
struct b3_mailbox { atomic_uint ready; int payload; };
void b3_mailbox_init(struct b3_mailbox *);
int b3_publish(struct b3_mailbox *,int value);
int b3_take(struct b3_mailbox *,int *out);
/* These order choices are used by both handoffs, allowing source-bound review. */
memory_order b3_load_order(void);
memory_order b3_store_order(void);
#endif
