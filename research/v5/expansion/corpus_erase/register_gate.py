import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
names=['author.py','resources.json','sources.md','reflection.md','taxonomy_refinement.json','register_gate.py']
data=dict(status='passage-inspected',scope='Exact selected primary C++ draft paragraphs and original C adaptation, not independent acceptance',resource_ids=[r['id'] for r in json.loads((HERE/'resources.json').read_text())['resources']],research_input_sha256={str((HERE/n).relative_to(ROOT)).replace('\\','/'):hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},limitations=['Current working draft is not published C++17 text.','Only named paragraphs inspected; actual C++17 implementation observations are separate.','No entire container objective or finite-test mastery claim.'])
(HERE/'research_gate.json').write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
