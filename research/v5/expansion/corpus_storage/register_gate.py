import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
names=['author.py','resources.json','sources.md','reflection.md','taxonomy_refinement.json','register_gate.py']
gate=dict(status='passage-inspected',scope='Selected primary passages supporting three bounded storage/index exercises, not independent acceptance or global corpus completeness',resource_ids=[r['id'] for r in json.loads((HERE/'resources.json').read_text())['resources']],research_input_sha256={str((HERE/n).relative_to(ROOT)).replace('\\','/'):hashlib.sha256((HERE/n).read_bytes()).hexdigest() for n in names},limitations=['Only exact named source passages inspected.','Btrfs web text read but direct private capture403; no local hash invented.','Published C17 text remains separately tracked.','RAM publication and hypothetical cuts do not establish physical durability.'])
(HERE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf8')
