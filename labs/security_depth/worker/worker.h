#ifndef SECURITY_DEPTH_WORKER_H
#define SECURITY_DEPTH_WORKER_H
#include <stddef.h>
#include <sys/types.h>
typedef struct {
  pid_t pid;
  int output;
} SDChild;
enum SDOperation { SD_READ_LOG, SD_WRITE_CONFIG };
/* Hosted Linux; zero or positive errno, output unchanged on failure. */
int sd_exclusive(int directory, const char *leaf, int *owned);
int sd_request(unsigned read_allowed, enum SDOperation, int borrowed_log,
               char out[4]);
/* Single-threaded parent, explicit executable/argv/env; fault0 normal,1..4
 * original finite acquisition failures. On success caller owns pid and output.
 */
int sd_spawn(const char *executable, char *const argv[], char *const env[],
             int borrowed_log, int borrowed_temp, unsigned fault, SDChild *out);
#endif
