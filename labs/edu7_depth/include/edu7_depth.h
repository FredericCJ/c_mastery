#ifndef EDU7_DEPTH_H
#define EDU7_DEPTH_H
#include <stddef.h>
#include <stdint.h>
enum {
  E7_OK = 0,
  E7_INVALID = 1,
  E7_BUSY = 2,
  E7_STALE = 3,
  E7_FULL = 4,
  E7_TIMEOUT = 5,
  E7_REJECTED = 6,
  E7_DUPLICATE = 7,
  E7_WAIT = 8,
  E7_START_TIMEOUT = 10,
  E7_END_TIMEOUT = 11,
  E7_UNIMPLEMENTED = 99
};
typedef struct {
  uint32_t last, turn_due, start_earliest, start_deadline, end_deadline,
      generation;
  uint8_t phase;
} E7Grant;
int e7_grant_begin(E7Grant *state, uint32_t now, uint32_t *token);
int e7_grant_first(E7Grant *state, uint32_t token, uint32_t now);
int e7_grant_finish(E7Grant *state, uint32_t token, uint32_t now);
int e7_grant_tick(E7Grant *state, uint32_t now);
int e7_link_admit(unsigned kind, unsigned local, unsigned destination,
                  unsigned granted, unsigned transmitting, unsigned uart_fault,
                  unsigned *deliver);
typedef struct {
  uint16_t id;
  uint8_t source, destination, index, count, credit, type, data[24], length;
} E7Fragment;
typedef struct {
  uint8_t bytes[120], length, index, count, source, destination, active,
      waiting, retries, credit, blocked, offered;
  uint16_t id;
  uint32_t last, due, blocked_since, offer_token;
  E7Fragment offer;
} E7Sender;
int e7_send_begin(E7Sender *state, uint16_t id, unsigned source,
                  unsigned destination, const uint8_t *bytes, size_t length,
                  uint32_t now);
int e7_send_offer(E7Sender *state, uint32_t now, E7Fragment *fragment,
                  uint32_t *token);
int e7_send_accept(E7Sender *state, uint32_t token, unsigned accepted,
                   uint32_t now);
int e7_send_ack(E7Sender *state, const E7Fragment *ack, uint32_t now);
int e7_send_expire(E7Sender *state, uint32_t now);
typedef struct {
  uint16_t checkpoint;
  uint8_t request[80], response[80], request_length, response_length;
} E7Receipt;
typedef struct {
  uint16_t session, epoch;
  uint8_t open, holder, token_pending, pending_direction, pending_length,
      pending[80];
  uint16_t pending_checkpoint;
  uint32_t generation, pending_token;
  E7Receipt receipt[2];
} E7Session;
int e7_session_open(E7Session *state, uint16_t session, uint16_t epoch,
                    unsigned schema, unsigned unit);
int e7_session_prepare(E7Session *state, unsigned sender, uint16_t session,
                       uint16_t epoch, uint16_t checkpoint,
                       const uint8_t *bytes, size_t length, uint32_t *token,
                       uint8_t *cached, size_t capacity, size_t *cached_length);
int e7_session_commit(E7Session *state, uint32_t token, unsigned accepted,
                      const uint8_t *response, size_t length);
int e7_token_begin(E7Session *state, unsigned sender, unsigned next);
int e7_token_complete(E7Session *state, unsigned next, unsigned confirmed);
int e7_session_resume(const E7Session *state, uint16_t session, uint16_t epoch,
                      unsigned sender, uint16_t checkpoint);
int e7_session_close(E7Session *state);
#endif
