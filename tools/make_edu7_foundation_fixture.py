from pathlib import Path
import hashlib,json
R=Path(__file__).resolve().parents[1];source=R/'research/enhancement/networking_contracts.json';d=json.loads(source.read_text(encoding='utf8'));f=next(f for f in d['families'] if f['domain']=='T40');out=R/'fixtures/edu7_foundation/original_t40.json';out.write_text(json.dumps(dict(source=source.relative_to(R).as_posix(),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),version=d['version'],family=f),indent=2)+'\n',encoding='utf8');print('PASS exact original T40 fixture extraction')
