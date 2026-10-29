#include "composition.h"
int erc_cause(struct erc_cause c,struct erc_error *o){(void)c;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_wrap(const struct erc_error *i,struct erc_cause c,struct erc_error *o){(void)i;(void)c;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_error(const struct erc_error *e,struct erc_result *o){(void)e;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_adopt(struct eop_owner *s,struct erc_result *o){(void)s;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_map_clone(const struct erc_result *s,erc_map_fn f,void *c,struct erc_result *o){(void)s;(void)f;(void)c;(void)o;return ERC_NOT_IMPLEMENTED;}
int erc_drop(struct erc_result *r){(void)r;return ERC_NOT_IMPLEMENTED;}
