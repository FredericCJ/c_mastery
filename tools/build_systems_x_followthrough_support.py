"""Exact original systems-X bridge; no global producer or inventory mutation."""
import argparse,copy,hashlib,importlib.util,json,re,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief,META
BASE='research/v5/expansion/systems_x_followthrough/';REPORTS=['reviews/v5/expansion/systems_x_followthrough/campaign_01.json','reviews/v5/expansion/systems_x_followthrough/controls_01.json','reviews/v5/expansion/systems_x_followthrough/target_export_01.json']
spec=importlib.util.spec_from_file_location('sx_bindings',ROOT/(BASE+'bindings.py'));binding=importlib.util.module_from_spec(spec);spec.loader.exec_module(binding);CLAUSES=binding.CLAUSES;PROVIDERS=binding.PROVIDERS
spec=importlib.util.spec_from_file_location('sx_attach',ROOT/(BASE+'author_support.py'));hook=importlib.util.module_from_spec(spec);spec.loader.exec_module(hook)
DIMS=('teaching','implementation','author_validation','external_review','independent_learner','physical_observation');PROFILES=('gcc-debug','gcc-release','clang-debug','clang-release','gcc-asan','clang-asan')
LIMITS=['Author source/reference execution, not independent review or unaided delayed learner assessment.','State models, actual host threads, isolated target execution and physical observations remain separate.','H745 dedicated-board heartbeat/reset/cache/DMA traces unobserved; exact published C17 and Crossbeam Guard detail source gaps retained.','Native C++ exception/call_once TSan comparison remains separately unvalidated in the retained once packet; no such claim is imported.','No universal progress, authentication, persistence, timing or WCET proof from finite passing experiments.']
def read(p):return json.loads((ROOT/p).read_text(encoding='utf-8-sig'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def write(p,v):
 path=ROOT/p;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n',encoding='utf8',newline='\n')
def section(body,name):
 m=re.search(r'^## '+re.escape(name)+r'\s*\n(.*?)(?=^## |\Z)',body,re.M|re.S);return m[1].strip() if m else ''
def variant(body,local):
 text=section(body,'Variants')
 if local in ['V1','V2']:
  m=re.search(r'(?<!\w)'+local+r':\s*(.*?)(?=\s+V[12]:|\n\n|\Z)',text,re.S);assert m;return local+': '+m[1].strip()
 m=re.search(r'Delayed:\s*(.*?)(?=\n\n|\Z)',text,re.S);assert m;return 'Delayed: '+m[1].strip()
def selectors(clause):return [name[1:] if name.startswith('@') else profile+' '+name for name in clause['checks'] for profile in ([''] if name.startswith('@') else PROFILES)]
def provider_rows(clause):
 result=[]
 for name,identity in clause['providers']:
  path=PROVIDERS[name];row=next(r for r in read(path)['updates'] if r['id']==identity);result.append(dict(provider=path,provider_sha256=sha(path),id=identity,row=copy.deepcopy(row)))
 return result
def dims(origin,providers):
 teaching={origin,'labs/systems_x_followthrough/README.md','labs/systems_x_followthrough/COSTS.md'};implementation={'assessor/systems_x_followthrough/ANALYSIS.md'};validation=set(REPORTS)
 for p in (ROOT/'labs/systems_x_followthrough').rglob('*.md'):teaching.add(p.relative_to(ROOT).as_posix())
 for p in (ROOT/'assessor/systems_x_followthrough').rglob('*'):
  if p.is_file() and p.suffix in ['.c','.py'] and '__pycache__' not in p.parts:implementation.add(p.relative_to(ROOT).as_posix())
 for item in providers:
  row=item['row'];teaching.update(row['dimensions']['teaching']['evidence']);implementation.update(row['dimensions']['implementation']['evidence']);validation.update(row['dimensions']['author_validation']['evidence'])
 return dict(teaching=dict(state='complete-exact-original-and-changed-condition-contracts',evidence=sorted(teaching)),implementation=dict(state='implemented-or-derived-exact-bounded-author-mechanisms',evidence=sorted(implementation)),author_validation=dict(state='passed-source-bound-original-and-changed-condition-checks',evidence=sorted(validation)),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-run-distinct-host-model-emulator-gates',evidence=['labs/systems_x_followthrough/README.md']))
def aggregate(children):return {key:dict(state=children[0]['dimensions'][key]['state'],evidence=sorted({p for row in children for p in row['dimensions'][key]['evidence']})) for key in DIMS}
def attach_cards():
 for identity in CLAUSES:
  path=ROOT/f'curriculum/v5/exercises/{identity}.md';raw=path.read_bytes();meta,body=parse_brief(path);history=ROOT/(BASE+'history/'+identity+'.md');history.parent.mkdir(parents=True,exist_ok=True)
  if not history.exists():history.write_bytes(raw)
  changed=hook.attach(meta);text=raw.decode('utf8');match=META.match(text);assert match
  path.write_text('```json\n'+json.dumps(changed,ensure_ascii=False,indent=2)+'\n```\n\n'+body,encoding='utf8',newline='\n')
def build():
 for report in REPORTS:assert read(report)['passed']
 rows=[];roots=[];pins=set(REPORTS);selected=set()
 for identity,clauses in CLAUSES.items():
  origin=f'curriculum/v5/exercises/{identity}.md';meta,body=parse_brief(ROOT/origin);old,oldbody=parse_brief(ROOT/(BASE+'history/'+identity+'.md'));stripped=copy.deepcopy(meta);stripped.pop('systems_x_followthrough_author_support',None);assert stripped==old and body==oldbody;children=[];delayed=meta.get('delayed_transfer',meta.get('delayed_assessment'));assert delayed['minimum_days']==7
  for criterion in meta['rubric']:
   key=criterion['id'].rsplit('-',1)[1];clause=clauses[key];providers=provider_rows(clause);row=dict(id=criterion['id'],original_source=origin,original_criterion=copy.deepcopy(criterion),closure_rationale=clause['rationale'],required_checks=selectors(clause),provider_bindings=providers,dimensions=dims(origin,providers),evidence_limits=LIMITS);children.append(row);rows.append(row)
  ids=[]
  for local in delayed['variant_ids']:
   key=local.rsplit('-',1)[-1];ident=local if local.startswith('X-') else identity+'-'+local;ids.append(ident);clause=clauses[key];providers=provider_rows(clause);row=dict(id=ident,original_source=origin,original_local_id=local,original_variant=variant(body,key),original_variant_section=section(body,'Variants'),minimum_days=7,closure_rationale=clause['rationale'],required_checks=selectors(clause),provider_bindings=providers,dimensions=dims(origin,providers),evidence_limits=LIMITS);children.append(row);rows.append(row)
  rows.append(dict(id=identity,original_source=origin,required_criteria=[r['id'] for r in meta['rubric']],required_variants=ids,original_immediate=section(body,'Variants') if identity in ['X-T51-002','X-T55-002','X-T55-003'] else None,closure_rationale='Exact constituent criteria and original variants; immediate clauses are concretely implemented by the named adapters.',dimensions=aggregate(children),evidence_limits=LIMITS))
  roots.append(dict(exercise=identity,source=origin,criteria=meta['rubric'],delayed=delayed,body_sha256=hashlib.sha256(body.encode()).hexdigest(),prerequisites=meta['prerequisite_exercises'],variant_section=section(body,'Variants')))
  pins.add(BASE+'history/'+identity+'.md')
  for row in children:selected.update(item['provider'] for item in row['provider_bindings'])
 # Exact modern parent origin bodies are retained; neither source nor independent
 # gaps are promoted by these software author dispositions.
 modern='curriculum/v5/expansion/modern_bridge_depth.json';original=read(modern)['remaining']
 parent_specs=[('MBR-ONCE-HISTORICAL',['X-T51-002-R1','X-T51-002-R2','X-T51-002-R3'],'Separate CDOnce permanent epoch corrects the original project milestone while raw CMX retry source/fixtures/receipts remain unchanged; explicit policies, failure cuts, native contenders and target table campaign.'),('MBR-TARGET-ADAPTERS',['X-T51-002-R2','X-T55-002-R2','X-T55-003-R2'],'Actual CMX blocking table adapter plus new synchronized three-node containers on actual M4 SVC/PendSV/SysTick contexts, explicit helper/link closure, native TSan and retained independent schedule expectations; no physical observation.')]
 for ident,child_ids,rationale in parent_specs:
  children=[next(r for r in rows if r['id']==i) for i in child_ids];row=dict(id=ident,original_source=modern,original_record=next(r for r in original if r['id']==ident),required_children=child_ids,closure_rationale=rationale,dimensions=aggregate(children),evidence_limits=LIMITS);rows.append(row)
 pins.add(modern)
 mapping=dict(schema_version=1,roots=roots,providers=sorted(selected),reports=REPORTS,scope='16 exact roots,42 original rubric objects,29 variants and2 explicitly bounded modern parents.')
 write(BASE+'evidence_map.json',mapping)
 for row in rows:
  for dimension in row['dimensions'].values():pins.update(dimension['evidence'])
 for report in REPORTS:pins.update(read(report)['input_sha256'])
 pins.update(selected);pins.update([BASE+'bindings.py',BASE+'author_support.py',BASE+'evidence_map.json',BASE+'intake.json',BASE+'audit.md','tools/build_systems_x_followthrough_support.py','tools/check_systems_x_followthrough.py','tools/check_residual_followthrough.py','tools/build_residual_followthrough_support.py','cmastery/exercises.py','research/v5/expansion/modern_bridge/resources.json','research/v5/expansion/modern_bridge/sources.md'])
 for p in pins:assert (ROOT/p).is_file(),p
 write(BASE+'support.json',dict(schema_version=1,required=True,updates=sorted(rows,key=lambda r:r['id']),source_sha256={p:sha(p) for p in sorted(pins)},current_reports=REPORTS,providers=sorted(selected),remaining_author_work=[],remaining_external_work=LIMITS,scope=mapping['scope']))
 print(json.dumps({'updates':len(rows),'providers':len(selected),'source_pins':len(pins)}))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--attach',action='store_true');a=p.parse_args()
 if a.attach:attach_cards()
 build()
