import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
names=['author.py','resources.json','sources.md','reflection.md','taxonomy_refinement.json','register_gate.py']
gate=dict(status='passage-inspected',scope='Three named text-composition mechanisms and selected primary passages; not global corpus completeness or independent acceptance',resource_ids=[r['id'] for r in json.loads((HERE/'resources.json').read_text())['resources']],research_input_sha256={str((HERE/n).relative_to(ROOT)).replace('\\','/'):hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},limitations=['Selected source passages only.','Oppen1980 is not independently inspected; Wadler is the inspected derivation source.','OWASP web passages read; direct private byte capture returned403.','No universal layout optimality, parser recovery or injection-safety claim.'])
(HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
