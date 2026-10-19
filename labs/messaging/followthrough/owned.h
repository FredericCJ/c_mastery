#ifndef CMASTERY_MESSAGING_OWNED_FOLLOW_H
#define CMASTERY_MESSAGING_OWNED_FOLLOW_H
#include "messaging.h"
#include "owned_value.h"
typedef struct { MfoValue value; bool used, send_done, delivered; } MfcRecord;
typedef struct {
  MsgHub hub;
  pthread_mutex_t records_mutex;
  MfcRecord records[8];
  unsigned active;
  bool retired;
} MfcHub;
typedef void (*MfcAfterSubmit)(void *);
MsgStatus mfc_init(MfcHub *, size_t, size_t);
MsgStatus mfc_send(MfcHub *, unsigned, MfoValue *, bool, MfcAfterSubmit, void *);
MsgStatus mfc_select(MfcHub *, unsigned, bool, unsigned *, MfoValue *);
MsgStatus mfc_close(MfcHub *, unsigned);
void mfc_cancel(MfcHub *);
MsgStatus mfc_destroy(MfcHub *);

typedef bool (*MfaHandler)(void *, const MfoValue *);
typedef void (*MfaDeadLetter)(void *, uint64_t, MfoValue);
typedef struct { MfoValue value; bool used; } MfaRecord;
typedef struct {
  MsgActor actor;
  MfaRecord records[5];
  MfaHandler handler;
  MfaDeadLetter dead;
  void *ctx, *dead_ctx;
} MfaActor;
MsgStatus mfa_init(MfaActor *, MfaHandler, void *, MfaDeadLetter, void *);
MsgStatus mfa_send(MfaActor *, uint64_t, MfoValue *);
MsgStatus mfa_step(MfaActor *);
MsgStatus mfa_restart(MfaActor *);
MsgStatus mfa_destroy(MfaActor *);
#endif
