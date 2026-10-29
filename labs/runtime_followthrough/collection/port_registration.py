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
_ANCHOR_TOKEN = re.compile(r"[A-Za-z_]\w*|\d+|[^\s]")


def anchor_spans(text, anchor):
    """Character spans where `anchor`'s tokens occur in order in `text`.

    Same rule as tools/c_source.py: braces in the source that the anchor
    lacks may be skipped only when they balance inside the match, so an
    anchor written for `if (x) stmt;` still finds `if (x) { stmt; }`.
    """
    hay = [(m.group(), m.start(), m.end()) for m in _ANCHOR_TOKEN.finditer(text)]
    needle = _ANCHOR_TOKEN.findall(anchor)
    spans = []
    for start, (token, _, _) in enumerate(hay):
        if not needle or token != needle[0]:
            continue
        j, depth, ok = start, 0, True
        for want in needle:
            while j < len(hay) and hay[j][0] != want and hay[j][0] in "{}":
                depth += 1 if hay[j][0] == "{" else -1
                if depth < 0:
                    ok = False
                    break
                j += 1
            if not ok or j >= len(hay) or hay[j][0] != want:
                ok = False
                break
            j += 1
        if not ok:
            continue
        while depth > 0 and j < len(hay) and hay[j][0] == "}":
            depth -= 1
            j += 1
        if depth == 0:
            spans.append((hay[start][1], hay[j - 1][2]))
    return spans


def spans(text,anchor,start=0):
 found=[]
 for lo,hi in anchor_spans(text,anchor):
  if lo<start:continue
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
