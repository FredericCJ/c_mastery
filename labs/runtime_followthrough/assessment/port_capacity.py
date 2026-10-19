"""Explicit CVM1-H8 resource profile, preserving original source/header."""
from pathlib import Path
import hashlib,json,sys
p=Path(sys.argv[1]);out=Path(sys.argv[2]);s=p.read_text();old='#define CVM_HEAP 16u';assert s.count(old)==1,'reinspect changed provider';out.parent.mkdir(parents=True,exist_ok=True);out.write_text(s.replace(old,'#define CVM_HEAP 8u'));out.with_suffix('.profile.json').write_text(json.dumps(dict(profile='CVM1-H8',source=str(p),source_sha256=hashlib.sha256(p.read_bytes()).hexdigest(),output_sha256=hashlib.sha256(out.read_bytes()).hexdigest(),changed_contract='8pair slots and sweep8; same wire grammar, different capacity/budget/representation; not baseline16'),indent=2)+'\n')
