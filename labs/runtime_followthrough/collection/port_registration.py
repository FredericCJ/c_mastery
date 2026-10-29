"""R-VG-struct or_vm-ENUM-2026-01: explicit, checked portable enum guard successor.

The original provider is never edited. enum cvm_tag has enumerators0..3. Its compatible
integer type may be unsigned; x.tag<0 then warns under GCC Arm. Conversion to
unsigned followed by comparison with3 rejects negative signed values as well as
all positive invalid values on these uint32/enum32 profiles. This does not relax
diagnostics, change layout, or discard invalid-tag checking.
"""
from pathlib import Path
import argparse,hashlib,json,re
OLD='if(x.tag<CVM_NIL||x.tag>CVM_REF)return 0;'
NEW='if((unsigned)x.tag>CVM_REF)return 0;'
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
def port(source,destination):
 raw=source.read_bytes();text=raw.decode('utf8')
 found=spans(text,OLD)
 if len(found)!=1:raise ValueError('Expected one exact retained struct or_vm enum guard; inspect a changed provider before porting')
 destination.write_text(text[:found[0][0]]+NEW+text[found[0][1]:],encoding='utf8',newline='\n')
 record=dict(id='R-VG-struct or_vm-ENUM-2026-01',source=str(source),source_sha256=hashlib.sha256(raw).hexdigest(),output=str(destination),output_sha256=hashlib.sha256(destination.read_bytes()).hexdigest(),old=OLD,new=NEW,scope='Only signedness-independent tag guard; all original provider operations retained')
 destination.with_suffix('.port.json').write_text(json.dumps(record,indent=2)+'\n');return destination
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('destination',type=Path);a=p.parse_args();port(a.source,a.destination);print('VG_ENUM_PORT_PASS exact_guard_successor')
