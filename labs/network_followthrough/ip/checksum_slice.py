"""Explicit author adapter: compile the unchanged checksum definitions alone.

The historical multi-mechanism translation unit includes an unrelated reducer
that GCC15 diagnoses at -O1/-O3. Do not suppress diagnostics or rewrite that
frozen provider. This bounded extraction is for that recorded source layout,
not a general C parser and not an implicit substitute for learner dependencies.
"""
from pathlib import Path
import argparse,hashlib,json
def extract(raw):
    starts=[b'int s_checksum_add(',b'int s_checksum_finish(',b'int s_paired(']
    if any(raw.count(s)!=1 for s in starts):raise ValueError('Unique recorded checksum boundaries required')
    a,b,c=[raw.index(s) for s in starts]
    if not a<b<c:raise ValueError('Recorded checksum order required')
    body=raw[a:c]
    if body.count(b'{')!=body.count(b'}') or body.count(b'int s_checksum_')!=2:raise ValueError('Complete two-definition range required')
    return b'#include "systems.h"\n'+body
def emit(source,destination):
    raw=source.read_bytes();out=extract(raw);destination.write_bytes(out)
    d=dict(source=str(source.resolve()),source_sha256=hashlib.sha256(raw).hexdigest(),output=str(destination.resolve()),output_sha256=hashlib.sha256(out).hexdigest(),scope='Exactly two unchanged original checksum definitions plus their original public declarations; no source repair or diagnostic suppression')
    destination.with_suffix('.identity.json').write_text(json.dumps(d,indent=2)+'\n');return d
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('source',type=Path);p.add_argument('destination',type=Path);a=p.parse_args();print(json.dumps(emit(a.source,a.destination)))
