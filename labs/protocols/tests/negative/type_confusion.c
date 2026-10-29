#include "protocols.h"

/* Required translation failure: wrappers differ; C still permits copying them. */
int wrong(struct pt_closed *closed, struct pt_closed *out)
{
    return pt_close(closed, out);
}
