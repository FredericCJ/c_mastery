#ifndef ARCHITECTURE_COMMIT_H
#define ARCHITECTURE_COMMIT_H
#include "systems_certificates.h"
#include "alu.h"
enum at_status { AT_OK,AT_INVALID,AT_PENDING,AT_CANCELLED,AT_NO_PENDING,AT_DONE,AT_SQUASHED,AT_STALE,AT_FAULT };
enum at_phase { AT_COMMIT,AT_ISSUE,AT_WAIT,AT_CANCEL,AT_BRANCH,AT_RETIRE };
struct at_machine { struct sc_machine state,pending_state;struct sc_control pending_control;uint32_t ticks;unsigned zero,pending,pending_zero; };
struct at_trace { struct sc_control wires;unsigned left,right,committed,phase;uint32_t ticks; };
struct at_spec { unsigned active,predicted_pc,candidate_valid,candidate_zero;struct sc_machine origin,branch,candidate;struct sc_control branch_control,candidate_control; };
enum at_status at_init(struct at_machine *,const struct sc_machine *,unsigned);
enum at_status at_input(struct at_machine *,unsigned,unsigned);
enum at_status at_begin(struct at_machine *,const struct sc_instruction *,size_t,struct at_trace *);
enum at_status at_poll(struct at_machine *,unsigned,unsigned,struct at_trace *);
enum at_status at_predict(const struct at_machine *,const struct sc_instruction *,size_t,unsigned,struct at_spec *);
enum at_status at_resolve(struct at_machine *,struct at_spec *,struct at_trace *);
enum at_status at_retire(struct at_machine *,struct at_spec *,struct at_trace *);
enum at_status at_discard(struct at_spec *);
int at_same(const struct sc_machine *,const struct sc_machine *);
int at_test(const char *);
void at_failure(const char *,unsigned);
void at_emit(const char *,const unsigned *,unsigned);
#endif
