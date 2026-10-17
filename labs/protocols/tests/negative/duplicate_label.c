#include "protocols.h"
/* Required translation failure, unlike an accidental __LINE__ collision. */
int wrong(PTThread *p,unsigned ready){PT_BEGIN(p);PT_WAIT(p,1,ready);PT_WAIT(p,1,ready);PT_END(p);}
