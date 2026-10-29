"""Add explicit host verification admission/count hooks to an exact source copy."""
import argparse,hashlib,pathlib,re
def spans(text,anchor,start=0):
 tokens=re.findall(r'[A-Za-z_]\w*|\d+|[^\s]',anchor);pattern=r'\s*'.join(re.escape(x) for x in tokens);found=[]
 if tokens[0][0].isalnum() or tokens[0][0]=='_':pattern=r'(?<!\w)'+pattern
 if tokens[-1][-1].isalnum() or tokens[-1][-1]=='_':pattern+=r'(?!\w)'
 for m in re.finditer(pattern,text):
  if m.start()<start:continue
  lo,hi=m.span()
  while anchor[:1] in (' ','\t') and text[lo-1:lo] in (' ','\t'):lo-=1
  if anchor.endswith('\n'):
   n=re.compile(r'[ \t]*\r?\n').match(text,hi);hi=n.end() if n else hi
  found.append((lo,hi))
 return found
def once_span(text,anchor):
 found=spans(text,anchor)
 if len(found)!=1:raise SystemExit(f'C anchor must occur exactly once ({len(found)}): '+anchor[:80])
 return found[0]
def once(text,old,new):
 lo,hi=once_span(text,old);return text[:lo]+new+text[hi:]
p=argparse.ArgumentParser();p.add_argument('--input',required=True);p.add_argument('--output',required=True);p.add_argument('--sha256',required=True);a=p.parse_args();raw=pathlib.Path(a.input).read_bytes();assert hashlib.sha256(raw).hexdigest()==a.sha256
s=raw.decode('utf8');s=once(s,'#include "store.h"','#include "store.h"\nextern int st_page_admit(uint32_t,int);\nextern void st_refresh_visit(uint32_t);')
needle='    if (!(s->used & (UINT32_C(1) << i))) {\n      s->used |= UINT32_C(1) << i;'
s=once(s,needle,'    if (!(s->used & (UINT32_C(1) << i))) {\n      if(!st_page_admit(i,leaf)) return 0;\n      s->used |= UINT32_C(1) << i;')
needle='  n = &s->scratch[id];\n  if (n->leaf) {\n    if (n->n) {'
s=once(s,needle,'  st_refresh_visit(id);\n  n = &s->scratch[id];\n  if (n->leaf) {\n    if (n->n) {')
pathlib.Path(a.output).write_text(s,encoding='utf8',newline='\n')
