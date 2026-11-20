#ifndef C_MASTERY_LINUX_SUPERVISION_H
#define C_MASTERY_LINUX_SUPERVISION_H

/* Hosted C17 on Linux. The guest fixture starts each entry as UID 1000,
 * PID 1, in a fresh PID/mount namespace and controlled cgroup v2 leaf.
 * A result of zero means the run produced its observation record.
 * A setup, I/O, protocol or cleanup error must return nonzero. */
int supervise_namespace(const char *variant); /* view */
int supervise_pids(const char *variant);      /* two, three */
int supervise_shutdown(const char *variant);  /* normal, early, startfail, stubborn */
int supervise_network(const char *variant);   /* A, B */
int supervise_telemetry(const char *variant); /* baseline, quota, stalled */

enum {
    SUPERVISION_BARRIER_FD = 3, /* network and telemetry. Send 'R', then receive 'G'. */
    SUPERVISION_PORT = 39073,
    SUPERVISION_FRAMES = 8192
};

#endif
