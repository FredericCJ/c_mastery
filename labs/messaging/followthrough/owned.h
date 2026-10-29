#ifndef CMASTERY_MESSAGING_OWNED_FOLLOW_H
#define CMASTERY_MESSAGING_OWNED_FOLLOW_H
#include "messaging.h"
#include "owned_value.h"
struct mfc_record { struct mfo_value value; bool used, send_done, delivered; };
struct mfc_hub {
  struct msg_hub hub;
  pthread_mutex_t records_mutex;
  struct mfc_record records[8];
  unsigned active;
  bool retired;
};
typedef void (*mfc_after_submit_fn)(void *);
enum msg_status mfc_init(struct mfc_hub *, size_t, size_t);
enum msg_status mfc_send(struct mfc_hub *, unsigned, struct mfo_value *, bool, mfc_after_submit_fn, void *);
enum msg_status mfc_select(struct mfc_hub *, unsigned, bool, unsigned *, struct mfo_value *);
enum msg_status mfc_close(struct mfc_hub *, unsigned);
void mfc_cancel(struct mfc_hub *);
enum msg_status mfc_destroy(struct mfc_hub *);

typedef bool (*mfa_handler_fn)(void *, const struct mfo_value *);
typedef void (*mfa_dead_letter_fn)(void *, uint64_t, struct mfo_value);
struct mfa_record { struct mfo_value value; bool used; };
struct mfa_actor {
  struct msg_actor actor;
  struct mfa_record records[5];
  mfa_handler_fn handler;
  mfa_dead_letter_fn dead;
  void *ctx, *dead_ctx;
};
enum msg_status mfa_init(struct mfa_actor *, mfa_handler_fn, void *, mfa_dead_letter_fn, void *);
enum msg_status mfa_send(struct mfa_actor *, uint64_t, struct mfo_value *);
enum msg_status mfa_step(struct mfa_actor *);
enum msg_status mfa_restart(struct mfa_actor *);
enum msg_status mfa_destroy(struct mfa_actor *);
#endif
