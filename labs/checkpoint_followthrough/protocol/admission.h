#ifndef CHECKPOINT_PROTOCOL_ADMISSION_H
#define CHECKPOINT_PROTOCOL_ADMISSION_H
#include "checkpoint_depth.h"
CPStatus cpa_admit(CPParser *,CPQueue *,size_t capacity);
#endif
