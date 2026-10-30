#ifndef CHECKPOINT_PROTOCOL_ADMISSION_H
#define CHECKPOINT_PROTOCOL_ADMISSION_H
#include "checkpoint_depth.h"

/*
 * Moves the parser's complete frame into the queue, honouring a smaller
 * occupancy limit than the physical queue (capacity <= CP_QUEUE).
 *
 * Ownership: the parser owns its frame until this call returns CP_OK; the
 * queue owns it afterwards. No memory is allocated.
 * Returns CP_INVALID for a null argument, capacity == 0, capacity > CP_QUEUE
 * or a queue already holding more than capacity items; CP_NEED_MORE when the
 * parser holds no complete frame; CP_FULL when the queue holds capacity items
 * (the frame stays in the parser, unchanged, to be offered again); otherwise
 * the result of cp_deliver(). Never blocks.
 */
enum cp_status cpa_admit(struct cp_parser *, struct cp_queue *,
                         size_t capacity);
#endif
