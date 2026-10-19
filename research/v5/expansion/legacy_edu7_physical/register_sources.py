"""Refresh only this inspected-source gate; no global registry rewrite."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
files=[HERE/n for n in ['resources.json','source_access.json','audit.md','register_sources.py']]
d=dict(status='passage-inspected',scope='T40.02 author implementation and selected source evidence; not independent content acceptance',resource_ids=[r['id'] for r in json.loads((HERE/'resources.json').read_text())['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Physical board/three-board bus observation not-run.','Full standards and held-book gaps remain; web-only DS9484 has no local captured hash.'])
(HERE/'research_gate.json').write_text(json.dumps(d,indent=2)+'\n')
