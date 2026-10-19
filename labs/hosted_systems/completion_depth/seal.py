"""Create a fresh immutable-by-convention pre-experiment identity record."""
from datetime import datetime,timezone
from pathlib import Path
import argparse,hashlib,json
p=argparse.ArgumentParser();p.add_argument('--prediction',required=True);p.add_argument('--implementation',required=True);p.add_argument('--output',required=True);a=p.parse_args()
prediction=Path(a.prediction);implementation=Path(a.implementation);raw=prediction.read_bytes();d=json.loads(raw)
if not all(isinstance(d.get(k),str) and len(d[k].strip())>=20 for k in ['prediction','changed_assumption','decision_rule']):raise ValueError('Supply a concrete prediction, changed assumption and decision rule before running alternate backend')
record=dict(recorded_utc=datetime.now(timezone.utc).isoformat(),prediction=d,input_sha256={str(prediction):hashlib.sha256(raw).hexdigest(),str(implementation):hashlib.sha256(implementation.read_bytes()).hexdigest()},limits='Creation timestamp is a local record, not trusted attestation or proof of reviewer independence. Preserve the original file and disclose earlier observations.')
with Path(a.output).open('x',encoding='utf8',newline='\n') as stream:stream.write(json.dumps(record,indent=2)+'\n')
