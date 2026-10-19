#ifndef ARCHITECTURE_COMMIT_H
#define ARCHITECTURE_COMMIT_H
#include "systems_certificates.h"
#include "alu.h"
enum ATStatus { AT_OK,AT_INVALID,AT_PENDING,AT_CANCELLED,AT_NO_PENDING,AT_DONE,AT_SQUASHED,AT_STALE,AT_FAULT };
enum ATPhase { AT_COMMIT,AT_ISSUE,AT_WAIT,AT_CANCEL,AT_BRANCH,AT_RETIRE };
typedef struct { SCMachine state,pending_state;SCControl pending_control;uint32_t ticks;unsigned zero,pending,pending_zero; } ATMachine;
typedef struct { SCControl wires;unsigned left,right,committed,phase;uint32_t ticks; } ATTrace;
typedef struct { unsigned active,predicted_pc,candidate_valid,candidate_zero;SCMachine origin,branch,candidate;SCControl branch_control,candidate_control; } ATSpec;
enum ATStatus at_init(ATMachine *,const SCMachine *,unsigned);
enum ATStatus at_input(ATMachine *,unsigned,unsigned);
enum ATStatus at_begin(ATMachine *,const SCInstruction *,size_t,ATTrace *);
enum ATStatus at_poll(ATMachine *,unsigned,unsigned,ATTrace *);
enum ATStatus at_predict(const ATMachine *,const SCInstruction *,size_t,unsigned,ATSpec *);
enum ATStatus at_resolve(ATMachine *,ATSpec *,ATTrace *);
enum ATStatus at_retire(ATMachine *,ATSpec *,ATTrace *);
enum ATStatus at_discard(ATSpec *);
int at_same(const SCMachine *,const SCMachine *);
int at_test(const char *);
void at_failure(const char *,unsigned);
void at_emit(const char *,const unsigned *,unsigned);
#endif
