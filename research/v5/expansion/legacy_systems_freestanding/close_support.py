"""Close only source-bound T11 author work; preserve original parents and receipts."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import read_json,dump,sha
from check_systems_freestanding import validate
from expansion_evidence import require_batch_research

folder=Path(__file__).parent;require_batch_research(folder)
receipt='reviews/v5/expansion/legacy_systems_freestanding/full_validation.json';check='reviews/v5/expansion/legacy_systems_freestanding/validation_check.json'
report=read_json(ROOT/receipt);errors=validate(report)
if errors:raise SystemExit(str(errors))
guard=read_json(ROOT/check)
if not guard.get('passed') or len(guard.get('negative_guards',[]))!=13 or guard.get('input_sha256',{}).get(receipt)!=sha(ROOT/receipt) or guard.get('input_sha256',{}).get('tools/check_systems_freestanding.py')!=sha(ROOT/'tools/check_systems_freestanding.py'):raise SystemExit('Current complete rejection guard required')
public='labs/machine_image/freestanding_depth/CONTRACTS.md';analysis='assessor/legacy_systems/freestanding_depth/ANALYSIS.md';support=folder/'support.json';relative=support.relative_to(ROOT).as_posix()
observations={(r['objective'],r['variant']):r for r in report['observations']};updates=[];criteria=[];canonical=[]
resources=read_json(folder/'resources.json')['resources'];source_records=[dict(id=r['id'],path='research/v5/expansion/legacy_systems_freestanding/resources.json',passages=r['locators']) for r in resources]
public_inputs=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/machine_image/freestanding_depth').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T11-*.json')):
 record=read_json(path);origin=path.relative_to(ROOT).as_posix();record['supplied_artifacts']=list(dict.fromkeys(record['supplied_artifacts']+public_inputs));existing={r['id'] for r in record['source_records']};record['source_records'] += [r for r in source_records if r['id'] not in existing]
 for criterion in record['criteria']:
  oid=criterion['objective'];criteria.append(dict(id=criterion['id'],objective=oid,project=record['project'],original_source=origin,task=criterion['task'],teaching=[public,origin],assessment_analysis=[analysis,record['assessor_analysis']],ordinary_observation=observations[oid,False],transfer_observation=observations[oid,True],independent_assessment='not-assessed; original minimum7days unchanged',receipt=receipt))
  for ident,variant in [(criterion['id'],False),*[(v['id'],True) for v in criterion['variants']]]:
   obs=observations[oid,variant];module=obs['group'].removeprefix('freestanding-')
   implementation=['assessor/legacy_systems/freestanding_depth/'+module+'.py']
   if oid.startswith('T11.01'):implementation.append('assessor/legacy_systems/freestanding_depth/manifests.py')
   updates.append(dict(id=ident,original_source=origin,original_objective=oid,project=record['project'],dimensions={
    'teaching':dict(state='complete-with-exact-published-ISO-reference-gap',evidence=[public,origin,analysis]),
    'implementation':dict(state='complete-for-declared-reference-contract',evidence=implementation),
    'author_validation':dict(state='passed-with-stated-scope',evidence=[receipt,check]),
    'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),
    'physical_observation':dict(state='not-observed; native-host-and-QEMU-evidence-only',evidence=[])},
    closure_rationale=obs['claim'],required_environment='Linux x86-64 GCC15/Clang21/binutils; private LLVM linker/newlib-nano; Arm GCC14 and QEMU10.2 MPS2 AN386. Exact versions/inputs in receipt.',validation_method='Separate per-leaf implementation/analysis and changed-condition execution; independent unbounded arithmetic/snapshot oracles, intended semantic/link/audit negatives, actual original prerequisite imports and final runtime closure.',evidence_limits=obs['limitations']+report['limits']))
  criterion['successor_support']=dict(path=relative,criterion_id=criterion['id'])
  if oid=='T11.04.c':criterion['versioned_correction']='research/v5/expansion/legacy_systems_freestanding/corrections.json#LSYS-T11-CARRY-2026-01'
 record['successor_support']=dict(path=relative,criterion_ids=[r['id'] for r in record['criteria']],scope='Source-bound author experiments and assessor transfer package; independent learner evidence remains separate.');dump(path,record);canonical.append(origin)
assert len(criteria)==30
parent=dict(id='LSYS-EXEC-T11',dimensions={k:dict(v) for k,v in updates[0]['dimensions'].items()},closure_rationale='All30 original ordinary and30 changed requirements have distinct mapped observations, actual source-bound implementations, five dependency manifests and13 rejection guards. Published ISO, non-author, learner and physical evidence remain separate.',evidence_limits=report['limits'],criteria=[r['id'] for r in criteria]);parent['dimensions']['implementation']['evidence']=['assessor/legacy_systems/freestanding_depth/'+m+'.py' for m in ('core','lowering','services','instrumentation','library','closure','targets','manifests')];updates.append(parent)
correction=dict(id='LSYS-T11-CARRY-2026-01',original_source='research/v5/expansion/legacy_systems_freestanding/corrections.json',original_objective='T11.04.c',dimensions={k:dict(v) for k,v in updates[0]['dimensions'].items()},closure_rationale='Ordinary prefix bound proves the high carry unreachable; active task/assessor corrected additively. Streaming arbitrary remainder exercises an actual129th bit. Accepted mi_divide and original authored snapshot remain unchanged.',evidence_limits=['Author derivation and bounded differential tests; no independent review.']);correction['dimensions']['implementation']['evidence']=['research/v5/expansion/legacy_systems/t11_cards.py','assessor/legacy_systems/freestanding_depth/divide.c'];updates.append(correction)
pins=[public,analysis,receipt,check,*canonical,'research/v5/expansion/legacy_systems_freestanding/resources.json','research/v5/expansion/legacy_systems_freestanding/provisioning.json','research/v5/expansion/legacy_systems_freestanding/corrections.json']
dump(support,dict(schema_version=1,required=True,scope='30 ordinary/30 changed T11 criteria and exact carry correction; no historical evidence rewritten',source_sha256={p:sha(ROOT/p) for p in pins},criteria=criteria,updates=updates,source_records=source_records))
dump(folder/'gap_ledger.json',dict(schema_version=1,required=True,gaps=[dict(id='LSYS-EXEC-T11',originating_record='research/v5/expansion/legacy_systems/gap_ledger.json',status='author-implemented-validated; external-dimensions-separate',requirement='All30 original T11 criteria and30 changed conditions',support=relative,remaining=['Non-author technical review','Independent ordinary and delayed attempts','Physical observations if extending the declared emulator/host target scope']),dict(id='LSYS-T11-CARRY-2026-01',originating_record='research/v5/expansion/legacy_systems_freestanding/corrections.json',status='active-teaching-and-assessment-corrected; author-verified',support=relative,requirement='Replace incorrect ordinary-division carry premise with invariant proof and meaningful streaming transfer'),dict(id='LSYS-T11-ISO',parent_id='LSYS-RUNTIME',status='source-access-gap',requirement='Exact published C17 and C23 passages supporting edition-level claims',current_evidence='N1570 and N3096 inspected as explicitly dated drafts only',completion='Inspect authorized published text and compare exact freestanding requirements; do not relabel drafts')],historical_note='Accepted providers, identities, hour records and historical receipts remain unchanged. Finite executions are not absolute mastery.'))
statepath=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(statepath);index={r['id']:r for r in state['updates']}
for row in updates:index[row['id']]=row
state['updates']=list(index.values());dump(statepath,state);print(dict(criteria=len(criteria),author_dispositions=len(updates),learner_claim=False))
