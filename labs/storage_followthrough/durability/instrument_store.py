"""Exact reviewed CST2 source adapter; never edits the retained provider."""
import argparse,hashlib,re
from pathlib import Path
EXPECTED='ff8f3b35057cfbd4e8834cd32c4b630cd66975289abe882ef4f9d0df2c549626'
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
def derive(source,slots=2):
 data=Path(source).read_bytes();assert hashlib.sha256(data).hexdigest()==EXPECTED
 s=data.decode('utf8').replace('\r\n','\n');old='  if (STORAGE_MUTANT_EARLY_RECLAIM) {'
 s='#include "guard.h"\n'+once(s,old,'  r=dg_validate(s,commit);\n  if(r)return r;\n'+old)
 assert slots in [2,3]
 if slots==3:
  def replace(old,new):
   nonlocal s
   s=once(s,old,new)
  replace('#define HOME 2u','#define HOME 3u')
  replace('#define MAGIC UINT32_C(0x43535432)','#define MAGIC UINT32_C(0x43535436)\n#ifndef STORAGE_MUTANT_THIRD_LOWEST\n#define STORAGE_MUTANT_THIRD_LOWEST 0\n#endif')
  replace('writep(s, (uint32_t)(gen & 1u), b)','writep(s, (uint32_t)(gen % 3u), b)')
  replace('  if (writep(&s, 0, b) || flush(&s) || writep(&s, 1, b) || flush(&s))\n    return STORE_IO;','  for(unsigned slot=0;slot<3;slot++)if(writep(&s,slot,b)||flush(&s))return STORE_IO;')
  replace('  uint8_t a[STORE_PAGE_SIZE], b[STORE_PAGE_SIZE], c[STORE_PAGE_SIZE];\n  int va, vb, r;','  uint8_t slots[3][STORE_PAGE_SIZE],c[STORE_PAGE_SIZE];\n  int chosen=-1,r;')
  begin=once_span(s,'  if (readp(s, 0, a) || readp(s, 1, b) || readp(s, COMMIT, c))')[0]
  end=spans(s,'  s->checkpoint_generation = s->generation;',begin)[0][0]
  s=s[:begin]+'''  for(unsigned i=0;i<3;i++)if(readp(s,i,slots[i]))return STORE_IO;
  if(readp(s,COMMIT,c))return STORE_IO;
  for(unsigned i=0;i<3;i++)if(valid(slots[i],20)&&rd32(slots[i]+16)==STORE_DATA_PAGES){
    uint64_t g=rd64(slots[i]+8);
    if(chosen<0||(STORAGE_MUTANT_THIRD_LOWEST?g<rd64(slots[chosen]+8):g>rd64(slots[chosen]+8)))chosen=(int)i;
  }
  if(chosen<0)return STORE_CORRUPT;
  s->generation=rd64(slots[chosen]+8);
'''+s[end:]
 return s
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('output');p.add_argument('--slots',type=int,choices=[2,3],default=2);p.add_argument('--sha256',action='store_true');a=p.parse_args();data=derive(a.source,a.slots).encode('utf8');Path(a.output).write_bytes(data)
 if a.sha256:print(hashlib.sha256(data).hexdigest())
