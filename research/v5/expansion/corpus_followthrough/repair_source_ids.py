"""Narrow collision correction; preserve old envelopes, never claim a new C run."""
import copy,hashlib,importlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4]
import sys
sys.path.insert(0,str(ROOT))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf-8')
for family,old,new,project in [('legacy_architecture_bytes','R-AB-ARM','R-ARCH-BYTES-ARMV7M','PR-T37-02'),('legacy_edu7_physical','R-EP-RM21','R-EDU7-PHYSICAL-RM21','PR-T40-02')]:
 folder=ROOT/'research/v5/expansion'/family
 for name in ['resources.json','research_gate.json','source_access.json','completion_dispositions.json','exports.json','validation.json']:
  p=folder/name
  if p.is_file():
   history=folder/('attempt_source_id_rename_'+name+'.txt')
   if not history.exists():history.write_bytes(p.read_bytes())
  if name in ['resources.json','source_access.json'] and p.is_file():p.write_text(p.read_text(encoding='utf-8').replace('"'+old+'"','"'+new+'"'),encoding='utf-8')
 gate=json.loads((folder/'research_gate.json').read_text(encoding='utf-8'));gate['resource_ids']=[new if x==old else x for x in gate['resource_ids']]
 gate['research_input_sha256']={n:sha(ROOT/n) for n in gate['research_input_sha256']};write(folder/'research_gate.json',gate)
 path=ROOT/'curriculum/v5/expansion/legacy_assessments'/f'{project}.json';before=json.loads(path.read_text(encoding='utf-8'));after=importlib.import_module('research.v5.expansion.'+family+'.author_support').attach(copy.deepcopy(before));assert after['criteria']==before['criteria'] and after['baseline_binding']==before['baseline_binding'];write(path,after)
 write(folder/'source_id_correction.json',dict(schema_version=1,old_id=old,new_id=new,reason='Distinct source/inspection identity collided in the global resource registry.',scope='Metadata-only rename; actual source passages and original experiment bytes are unchanged.',original_receipts_preserved=True,publication_record=str(path.relative_to(ROOT)).replace('\\','/'),export_and_package_refresh='Fresh isolated exports executed; current exports.json and completion_dispositions.json bind the renamed source and unchanged original experiment.' if '--finalize' in sys.argv else 'Required; no fresh export execution is inferred from this rename.',pins={str(p.relative_to(ROOT)).replace('\\','/'):sha(p) for p in [folder/'resources.json',folder/'research_gate.json',folder/'author_support.py',path]}))
 print('PASS source ID collision correction',family,new)
