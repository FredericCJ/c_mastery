#ifndef CHECKPOINT_PROTOCOL_ADMISSION_H
#define CHECKPOINT_PROTOCOL_ADMISSION_H
#include "checkpoint_depth.h"
enum cp_status cpa_admit(struct cp_parser *,struct cp_queue *,size_t capacity);
#endif
