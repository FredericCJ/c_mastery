#ifndef CHECKPOINT_DEPTH_H
#define CHECKPOINT_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum { CP_PAYLOAD = 32, CP_WIRE = 42, CP_QUEUE = 2 };
typedef enum { CP_OK, CP_NEED_MORE, CP_FRAME, CP_REJECT, CP_FULL, CP_INVALID } CPStatus;
typedef struct { uint16_t sequence; size_t length; unsigned char payload[CP_PAYLOAD]; } CPMessage;
typedef struct {
    unsigned char candidate[CP_WIRE];
    size_t used;
    int ready;
    CPMessage message;
} CPParser;
typedef struct { CPMessage items[CP_QUEUE]; size_t head, count; } CPQueue;
uint32_t cp_crc32(const unsigned char *, size_t);
CPStatus cp_encode(const CPMessage *, unsigned char *, size_t, size_t *);
void cp_parser_init(CPParser *);
CPStatus cp_feed(CPParser *, const unsigned char *, size_t, size_t *);
CPStatus cp_peek(const CPParser *, CPMessage *);
void cp_queue_init(CPQueue *);
CPStatus cp_deliver(CPParser *, CPQueue *);
CPStatus cp_take(CPQueue *, CPMessage *);
#endif
