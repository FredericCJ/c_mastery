#include "protocols.h"
/* Required translation failure: wrappers differ; C still permits copying them. */
int wrong(PTClosed *closed,PTClosed *out){return pt_close(closed,out);}
