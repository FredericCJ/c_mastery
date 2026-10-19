"""R-VG-ORVM-ENUM-2026-01: explicit, checked portable enum guard successor.

The original provider is never edited. CvmTag has enumerators0..3. Its compatible
integer type may be unsigned; x.tag<0 then warns under GCC Arm. Conversion to
unsigned followed by comparison with3 rejects negative signed values as well as
all positive invalid values on these uint32/enum32 profiles. This does not relax
diagnostics, change layout, or discard invalid-tag checking.
"""
from pathlib import Path
import argparse,hashlib,json
OLD='if(x.tag<CVM_NIL||x.tag>CVM_REF)return 0;'
NEW='if((unsigned)x.tag>CVM_REF)return 0;'
def port(source,destination):
 raw=source.read_bytes();text=raw.decode('utf8')
 if text.count(OLD)!=1:raise ValueError('Expected one exact retained ORVM enum guard; inspect a changed provider before porting')
 destination.write_text(text.replace(OLD,NEW),encoding='utf8',newline='\n')
 record=dict(id='R-VG-ORVM-ENUM-2026-01',source=str(source),source_sha256=hashlib.sha256(raw).hexdigest(),output=str(destination),output_sha256=hashlib.sha256(destination.read_bytes()).hexdigest(),old=OLD,new=NEW,scope='Only signedness-independent tag guard; all original provider operations retained')
 destination.with_suffix('.port.json').write_text(json.dumps(record,indent=2)+'\n');return destination
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('destination',type=Path);a=p.parse_args();port(a.source,a.destination);print('VG_ENUM_PORT_PASS exact_guard_successor')
