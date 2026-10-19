#include "composition.h"
int erc_cause(ErcCause c,ErcError *o){(void)c;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_wrap(const ErcError *i,ErcCause c,ErcError *o){(void)i;(void)c;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_error(const ErcError *e,ErcResult *o){(void)e;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_adopt(EopOwner *s,ErcResult *o){(void)s;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_map_clone(const ErcResult *s,ErcMap f,void *c,ErcResult *o){(void)s;(void)f;(void)c;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_drop(ErcResult *r){(void)r;return ERC_NOT_IMPLEMENTED;}
