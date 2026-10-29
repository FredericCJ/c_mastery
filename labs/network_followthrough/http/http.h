#ifndef NH_MCU_HTTP_H
#define NH_MCU_HTTP_H
#include "driver.h"
enum { NH_OK, NH_WAIT, NH_DONE, NH_ABORT, NH_INVALID, NH_STUB };
enum {
  NH_HANDSHAKE,
  NH_PARSE,
  NH_INTERIM,
  NH_FINAL,
  NH_CLOSING,
  NH_FINISHED,
  NH_FAILED
};
typedef const char *(*nh_date_fn)(void *);
struct nh_http {
  struct nb_net *net;
  struct http_parser parser;
  unsigned char response[1600];
  size_t length;
  nh_date_fn date;
  void *date_context;
  uint32_t epoch;
  unsigned state, offered, requests;
  uint64_t now, started, progress, output_started;
};
/* One request per connection, explicit Connection:close. A Date callback is
 * trusted UTC authority, never the transport clock. NULL omits Date. It must
 * return NULL or29 printable bytes plusNUL, valid until the call completes,
 * without reentry. No allocation. Dependencies and state storage stay live. */
int nh_init(struct nh_http *, struct nb_net *, const char *authority, nh_date_fn, void *,
            uint64_t now);
/* Serialized step invokes nb_step then consumes at most64 request bytes.
 * Request15s total/5s idle; interim/final output15s each. Absolute deadlines.
 * DONE ends application processing; TCP TIME_WAIT still needs nb_step.
 * ABORT does not assert device quiescence or release borrowed DMA storage. */
int nh_step(struct nh_http *, uint64_t now);
#endif
