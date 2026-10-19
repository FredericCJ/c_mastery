"""Host-only build identity utility; no target hashing or schema-operation library.

Identity binds the complete field-list bytes and this exact generator source.
Generated C derives semantic operations separately from MPS_FIELDS.
"""
import argparse
import hashlib
from pathlib import Path
def generate(source):
    data=Path(source).read_bytes();identity=hashlib.sha256(Path(__file__).read_bytes()+b'\0'+data).hexdigest()
    lines=['#ifndef CMASTERY_MPS_STAMP_H','#define CMASTERY_MPS_STAMP_H','#include <stdint.h>']
    lines+=['#define MPS_SCHEMA_'+str(i)+' UINT64_C(0x'+identity[i*16:(i+1)*16]+')' for i in range(4)]
    lines+=['#define MPS_SCHEMA_HEX "'+identity+'"','#define MPS_LINK_GUARD mps_schema_'+identity,'#endif','']
    return '\n'.join(lines)
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('output');p.add_argument('--check',action='store_true');a=p.parse_args();text=generate(a.source);dest=Path(a.output)
    if a.check:
        if not dest.is_file() or dest.read_text(encoding='utf8')!=text:raise SystemExit('MPS_SCHEMA_STALE generated identity differs from actual complete inputs')
    else:dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(text,encoding='utf8',newline='\n')
