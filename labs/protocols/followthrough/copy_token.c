#include "protocol_follow.h"

/* Translation succeeds: C copies the wrapper; runtime live-owner checks differ. */
struct pf_recovery copied(struct pf_recovery token)
{
    struct pf_recovery alias = token;
    return alias;
}
