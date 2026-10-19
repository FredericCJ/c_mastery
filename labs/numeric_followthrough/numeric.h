#ifndef NUMERIC_FOLLOWTHROUGH_H
#define NUMERIC_FOLLOWTHROUGH_H
typedef enum {NF_TELEMETRY,NF_AUDIT} NfPolicy;
typedef struct {double absolute,scaled;int accepted;} NfError;
/* Live disjoint output; finite inputs in +/-1e100. Refusal preserves output.
 * Telemetry: |got-want| <= 2^-40 + 2^-40*|want|.
 * Audit: identical binary64 object representations, including signed zero. */
int nf_compare(NfPolicy,double want,double got,NfError *);
#endif
