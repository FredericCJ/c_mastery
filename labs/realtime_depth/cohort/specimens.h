#ifndef CMASTERY_REALTIME_COHORT_SPECIMENS_H
#define CMASTERY_REALTIME_COHORT_SPECIMENS_H
/* Original intentionally flawed teaching specimen, not a deployable server.
 * Only the bounded scripted traces in the supplied driver are admitted. */
typedef struct {
 unsigned q,p,remaining,begin,usage,active,count;
 unsigned at[16],amount[16];
} SUFlawed;
void su_flawed_init(SUFlawed *,unsigned,unsigned);
void su_flawed_update(SUFlawed *,unsigned);
void su_flawed_active(SUFlawed *,unsigned,unsigned);
void su_flawed_run(SUFlawed *,unsigned,unsigned);
#endif
