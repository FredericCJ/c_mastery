"""Pin the four index source passages separately from the evolving discovery map."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
files=[HERE/n for n in ['resources.json','sources.md','reflection.md','source_access.json','taxonomy_refinement.json','author_indices.py','register_gate.py']]
gate=dict(status='passage-inspected',scope='Four named original index mechanisms; evolving metadata relevance screening is separately identified in screening_map.json and is not primary-passage evidence.',resource_ids=[r['id'] for r in json.loads((HERE/'resources.json').read_text())['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['The exact final C17 publication remains an existing separate source gap.','Publisher Fenwick erratum was browser-inspected; direct private capture403 is retained.','This bounded gate does not close corpus-wide discovery, external review, learner transfer or physical observation.'])
(HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n')
