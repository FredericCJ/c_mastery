import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
names=['author.py','resources.json','sources.md','reflection.md','taxonomy_refinement.json','register_gate.py']
data=dict(status='passage-inspected',scope='Selected W3C primary passages and original bounded adaptation; not independent acceptance',resource_ids=['R-CESC-XML08'],research_input_sha256={str((HERE/n).relative_to(ROOT)).replace('\\','/'):hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},limitations=['Only named sections inspected.','ASCII pair is an original restricted grammar, not XML/HTML conformance.','Actual component source observations and execution are separate from specification inspection.'])
(HERE/'research_gate.json').write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
