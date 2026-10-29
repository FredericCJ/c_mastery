#ifndef CMASTERY_PMU_CONTROL_H
#define CMASTERY_PMU_CONTROL_H
/* Supplied perf-tool protocol adapter. Environment names descriptor numbers:
 * PF_PERF_CONTROL_FD writes commands; PF_PERF_ACK_FD receives the five bytes
 * ack\n\0 (perf's evlist__ctlfd_ack includes the string terminator).
 * The caller owns descriptors. Failure never authorizes the workload boundary. */
int pf_perf_control(int enable);
#endif
