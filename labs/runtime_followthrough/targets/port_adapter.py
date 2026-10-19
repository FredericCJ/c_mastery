"""Exact explicit callback-policy successor; portable image and VM unchanged."""
from pathlib import Path
import argparse,hashlib,json
p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();raw=a.source.read_bytes();text=raw.decode('utf8')
changes={'*out = input + 1;':'*out = input + 2;', 'out.integer != 42':'out.integer != 43'}
for before,after in changes.items():
 if text.count(before)!=1:raise SystemExit('VX_PORT_REFUSED exact callback contract changed')
 text=text.replace(before,after)
a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(text,encoding='utf8',newline='\n')
print(json.dumps(dict(registration='R-VX-HOST-PLUS2-2026-01',original_sha256=hashlib.sha256(raw).hexdigest(),output_sha256=hashlib.sha256(a.output.read_bytes()).hexdigest(),changes=changes,scope='Explicit native callback policy +1 to +2; same VM and image, separate probe successor. Not a kernel provider replacement.')))
