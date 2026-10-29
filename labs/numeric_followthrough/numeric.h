#ifndef NUMERIC_FOLLOWTHROUGH_H
#define NUMERIC_FOLLOWTHROUGH_H

enum nf_policy
{
    NF_TELEMETRY,
    NF_AUDIT
};

struct nf_error
{
    double absolute, scaled;
    int accepted;
};

/* Live disjoint output; finite inputs in +/-1e100. Refusal preserves output.
 * Telemetry: |got-want| <= 2^-40 + 2^-40*|want|.
 * Audit: identical binary64 object representations, including signed zero. */
int nf_compare(enum nf_policy, double want, double got, struct nf_error *);
#endif
