#include "protocol_follow.h"
/* Translation succeeds: C copies the wrapper; runtime live-owner checks differ. */
PFRecovery copied(PFRecovery token){PFRecovery alias=token;return alias;}
