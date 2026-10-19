#ifndef LW_WRAPPERS_H
#define LW_WRAPPERS_H
typedef struct {unsigned left,right,calls;int value;char text[32];} LwObservation;
int lw_trace(unsigned,LwObservation *);
int lw_min(unsigned,LwObservation *);
int lw_statement(unsigned,LwObservation *);
#endif
