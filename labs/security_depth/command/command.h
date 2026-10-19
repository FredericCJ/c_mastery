#ifndef SECURITY_DEPTH_COMMAND_H
#define SECURITY_DEPTH_COMMAND_H
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
enum sc_result {
  SC_OK,
  SC_LENGTH,
  SC_OPCODE,
  SC_COUNT,
  SC_DENIED,
  SC_BUDGET,
  SC_STALE,
  SC_FULL,
  SC_INVALID,
  SC_EXHAUSTED,
  SC_UNIMPLEMENTED
};
typedef struct {
  unsigned generation, allowed;
} SCAuthority;
typedef struct {
  unsigned opcode, count, generation;
  uint16_t values[4];
} SCCommand;
typedef struct {
  unsigned count, work;
  SCCommand commands[4];
} SCBatch;
typedef struct {
  unsigned headers, values, bytes;
} SCMeter;
enum sc_result sc_decode(const unsigned char *, size_t, const SCAuthority *,
                         int, SCBatch *, SCMeter *);
enum sc_result sc_revoke(SCAuthority *);
enum sc_result sc_apply(const SCBatch *, const SCAuthority *, uint32_t *,
                        uint32_t *);
enum sc_result sc_decimal(const unsigned char *, size_t, int, unsigned *,
                          char[2]);
enum sc_result sc_extent(size_t, size_t, size_t, size_t, size_t *);
/* Original intentionally flawed authority target; synthetic counter only. */
enum sc_result sc_flawed_authority(const unsigned char *, size_t, uint32_t *);
typedef struct {
  unsigned index, generation;
} SCTicket;
typedef struct {
  unsigned active, generation, retries;
  SCBatch request;
} SCJob;
typedef struct {
  SCJob jobs[3];
  unsigned used, count, terminal, released, control_pending, closed;
} SCService;
void sc_service_init(SCService *);
enum sc_result sc_submit(SCService *, const SCBatch *, SCTicket *);
enum sc_result sc_finish(SCService *, SCTicket, int, const SCAuthority *,
                         uint32_t *, uint32_t *);
enum sc_result sc_retry(SCService *, SCTicket);
enum sc_result sc_control(SCService *);
enum sc_result sc_control_complete(SCService *);
enum sc_result sc_shutdown(SCService *);
typedef struct {
  int (*acquire)(void *, unsigned);
  void (*release)(void *, unsigned);
  void *context;
} SCResources;
typedef struct {
  unsigned state, attempts, owned, log_fallbacks;
} SCRecovery;
enum sc_result sc_recover(SCRecovery *, SCResources, int);
#endif
