"""Add explicit host verification admission/count hooks to an exact source copy."""
import argparse,hashlib,pathlib
p=argparse.ArgumentParser();p.add_argument('--input',required=True);p.add_argument('--output',required=True);p.add_argument('--sha256',required=True);a=p.parse_args();raw=pathlib.Path(a.input).read_bytes();assert hashlib.sha256(raw).hexdigest()==a.sha256
s=raw.decode('utf8');s=s.replace('#include "store.h"','#include "store.h"\nextern int st_page_admit(uint32_t,int);\nextern void st_refresh_visit(uint32_t);')
needle='    if (!(s->used & (UINT32_C(1) << i))) {\n      s->used |= UINT32_C(1) << i;';assert s.count(needle)==1
s=s.replace(needle,'    if (!(s->used & (UINT32_C(1) << i))) {\n      if(!st_page_admit(i,leaf)) return 0;\n      s->used |= UINT32_C(1) << i;')
needle='  n = &s->scratch[id];\n  if (n->leaf) {\n    if (n->n) {';assert s.count(needle)==1
s=s.replace(needle,'  st_refresh_visit(id);\n  n = &s->scratch[id];\n  if (n->leaf) {\n    if (n->n) {')
pathlib.Path(a.output).write_text(s,encoding='utf8',newline='\n')
