#ifndef CMASTERY_REALTIME_COHORT_SPECIMENS_H
#define CMASTERY_REALTIME_COHORT_SPECIMENS_H

/* Original intentionally flawed teaching specimen, not a deployable server.
 * Only the bounded scripted traces in the supplied driver are admitted. */
struct su_flawed
{
    unsigned q, p, remaining, begin, usage, active, count;
    unsigned at[16], amount[16];
};

void su_flawed_init(struct su_flawed *, unsigned, unsigned);
void su_flawed_update(struct su_flawed *, unsigned);
void su_flawed_active(struct su_flawed *, unsigned, unsigned);
void su_flawed_run(struct su_flawed *, unsigned, unsigned);
#endif
