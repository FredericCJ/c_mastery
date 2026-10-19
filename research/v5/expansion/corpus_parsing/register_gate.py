import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
names=['author.py','resources.json','sources.md','reflection.md','taxonomy_refinement.json','register_gate.py','make_stub.py']
gate=dict(status='passage-inspected',scope='Four named parser mechanisms and exact primary passages; not global corpus completeness or independent acceptance',resource_ids=[r['id'] for r in json.loads((HERE/'resources.json').read_text())['resources']],research_input_sha256={str((HERE/n).relative_to(ROOT)).replace('\\','/'):hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},limitations=['Selected passages, not entire papers/courses.','Exact published C17 remains separately tracked.','Bounded languages/DAG/table capacities are not universal parser conformance.'])
(HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
