#ifndef TD_TYPES_H
#define TD_TYPES_H
#include <stddef.h>
typedef enum {
  TD_OK,
  TD_INVALID,
  TD_RANGE,
  TD_BUSY,
  TD_STATE,
  TD_UNFINISHED
} TdStatus;
typedef int TdRow[4];
typedef int TdUnary(int);
typedef TdUnary *TdUnaryPtr;
typedef struct {
  TdUnaryPtr call;
} TdCallable;
int (*td_row_factory(void))[4];
TdUnaryPtr td_factory(void);
TdCallable td_wrapped_factory(void);
struct TdOpaque;
struct TdOpaque *td_owner(unsigned index);
TdStatus td_get(const struct TdOpaque *owner, int *out);
TdStatus td_add(struct TdOpaque *owner, int delta);
typedef struct {
  int value;
} TdMeters;
typedef struct {
  int value;
} TdSeconds;
typedef struct {
  int value;
} TdSpeed;
TdStatus td_speed(TdMeters distance, TdSeconds duration, TdSpeed *out);
TdStatus td_distance(TdSpeed speed, TdSeconds duration, TdMeters *out);
typedef struct {
  int bias;
  unsigned calls;
} TdContext;
int td_step(int value, void *context);
int td_adapter(void *context, int value);
typedef int (*TdDispatch)(void *, int);
TdStatus td_rows(const int (*rows)[4], size_t count, TdDispatch callback,
                 void *context, int *out);
typedef enum {
  TD_IDLE,
  TD_PENDING,
  TD_RUNNING,
  TD_DONE,
  TD_CANCELED
} TdJobState;
typedef struct {
  TdJobState state;
  TdDispatch callback;
  void *context;
  int argument;
} TdJob;
#define TD_JOB_INIT {TD_IDLE, NULL, NULL, 0}
TdStatus td_submit(TdJob *job, TdDispatch callback, void *context,
                   int argument);
TdStatus td_run(TdJob *job, int *out);
TdStatus td_cancel(TdJob *job);
TdStatus td_reset(TdJob *job);
#endif
