import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
names=['author.py','resources.json','sources.md','reflection.md','taxonomy_refinement.json','register_gate.py']
gate=dict(status='passage-inspected',scope='X-T17-003 bounded primary interval passages and original C adaptation; not independent acceptance',resource_ids=['R-CINTERVAL-SAND2007'],research_input_sha256={str((HERE/n).relative_to(ROOT)).replace('\\','/'):hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},limitations=['Only printed pp19-21 assigned passage inspected.','Exact published C17 text remains separately tracked.','Physical model correctness, independent learner assessment and floating interval-standard conformance are not inferred.'])
(HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf-8')
