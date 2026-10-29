#ifndef CHECKPOINT_DEPTH_H
#define CHECKPOINT_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum { CP_PAYLOAD = 32, CP_WIRE = 42, CP_QUEUE = 2 };
enum cp_status { CP_OK, CP_NEED_MORE, CP_FRAME, CP_REJECT, CP_FULL, CP_INVALID };
struct cp_message { uint16_t sequence; size_t length; unsigned char payload[CP_PAYLOAD]; };
struct cp_parser {
    unsigned char candidate[CP_WIRE];
    size_t used;
    int ready;
    struct cp_message message;
};
struct cp_queue { struct cp_message items[CP_QUEUE]; size_t head, count; };
uint32_t cp_crc32(const unsigned char *, size_t);
enum cp_status cp_encode(const struct cp_message *, unsigned char *, size_t, size_t *);
void cp_parser_init(struct cp_parser *);
enum cp_status cp_feed(struct cp_parser *, const unsigned char *, size_t, size_t *);
enum cp_status cp_peek(const struct cp_parser *, struct cp_message *);
void cp_queue_init(struct cp_queue *);
enum cp_status cp_deliver(struct cp_parser *, struct cp_queue *);
enum cp_status cp_take(struct cp_queue *, struct cp_message *);
#endif
