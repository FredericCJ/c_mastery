"""Bind exact T10 author evidence without changing accepted or learner records."""
from pathlib import Path
import sys
ROOT=Path(__file__).resolve().parents[4];sys.path.insert(0,str(ROOT/'tools'))
from common import read_json,dump,sha
from check_systems_startup import validate
from expansion_evidence import require_batch_research
folder=Path(__file__).parent;require_batch_research(folder)
receipt='reviews/v5/expansion/legacy_systems_startup/full_validation.json';check='reviews/v5/expansion/legacy_systems_startup/validation_check.json'
report=read_json(ROOT/receipt);errors=validate(report)
if errors:raise SystemExit(str(errors))
guard=read_json(ROOT/check)
if not guard.get('passed') or guard.get('input_sha256',{}).get(receipt)!=sha(ROOT/receipt):raise SystemExit('Current rejection-guard receipt required')
public='labs/machine_image/startup_depth/CONTRACTS.md';analysis='assessor/legacy_systems/startup_depth/ANALYSIS.md';support=folder/'support.json';relative=support.relative_to(ROOT).as_posix()
observations={(r['objective'],r['variant']):r for r in report['observations']};updates=[];criteria=[];canonical=[]
resources=read_json(folder/'resources.json')['resources'];source_records=[dict(id=r['id'],path='research/v5/expansion/legacy_systems_startup/resources.json',passages=r['locators']) for r in resources]
public_inputs=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/machine_image/startup_depth').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
for path in sorted((ROOT/'curriculum/v5/expansion/legacy_assessments').glob('PR-T10-*.json')):
 record=read_json(path);origin=path.relative_to(ROOT).as_posix();record['supplied_artifacts']=list(dict.fromkeys(record['supplied_artifacts']+public_inputs));existing={r['id'] for r in record['source_records']};record['source_records'] += [r for r in source_records if r['id'] not in existing]
 for criterion in record['criteria']:
  oid=criterion['objective'];criteria.append(dict(id=criterion['id'],objective=oid,project=record['project'],original_source=origin,task=criterion['task'],teaching=[public,origin],assessment_analysis=[analysis,record['assessor_analysis']],ordinary_observation=observations[oid,False],transfer_observation=observations[oid,True],independent_assessment='not-assessed; original minimum7days unchanged',receipt=receipt))
  for ident,variant in [(criterion['id'],False),*[(v['id'],True) for v in criterion['variants']]]:
   obs=observations[oid,variant];module={'startup-core':'core','startup-entry':'entry','startup-runtime':'runtime'}[obs['group']]
   updates.append(dict(id=ident,original_source=origin,original_objective=oid,project=record['project'],dimensions={
    'teaching':dict(state='complete-with-exact-published-ISO-reference-gap',evidence=[public,origin,analysis]),
    'implementation':dict(state='complete-for-declared-reference-contract',evidence=['assessor/legacy_systems/startup_depth/'+module+'.py']),
    'author_validation':dict(state='passed-with-stated-scope',evidence=[receipt,check]),
    'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-assessed',evidence=[]),
    'physical_observation':dict(state='not-applicable-to-declared-host-or-compile-only-contract',evidence=[])},
    closure_rationale=obs['claim'],required_environment='Linux x86-64 GCC/Clang/binutils/GDB; private musl/strace; Arm GCC/Clang cross-assembly only.',validation_method='Criterion-specific real entry/CRT/syscall/transfers plus independent bounded core oracle, actual forbidden-origin/control failures and source/binary identities.',evidence_limits=obs['limitations']+report['limits']))
  criterion['successor_support']=dict(path=relative,criterion_id=criterion['id'])
 record['successor_support']=dict(path=relative,criterion_ids=[r['id'] for r in record['criteria']],scope='Source-bound author experiments and assessor transfers; independent learner evidence remains separate.');dump(path,record);canonical.append(origin)
parent=dict(id='LSYS-EXEC-T10',dimensions={k:dict(v) for k,v in updates[0]['dimensions'].items()},closure_rationale='All30 original ordinary and30 changed requirements have separate mapped observations and implemented assessor support.',evidence_limits=report['limits'],criteria=[r['id'] for r in criteria]);parent['dimensions']['implementation']['evidence']=['assessor/legacy_systems/startup_depth/'+m+'.py' for m in ('core','entry','runtime')];updates.append(parent)
updates.append(dict(id='LSYS-RUNTIME-HOST-243',original_source='research/v5/expansion/legacy_systems_elf/gap_ledger.json',parent_id='LSYS-RUNTIME',closure_rationale='Authorized matching Ubuntu glibc-source2.43-2ubuntu2.4 package obtained; named startup, static-PIE, TLS and DSO-init passages inspected with package/body/patch hashes. Earlier2.42 evidence stays historical.',dimensions={
 'teaching':dict(state='complete-for-named-source-version-correlation',evidence=['research/v5/expansion/legacy_systems_startup/resources.json']),
 'implementation':dict(state='not-applicable-source-access-obligation',evidence=['research/v5/expansion/legacy_systems_startup/provisioning.json']),
 'author_validation':dict(state='verified-source-access-and-bounded-inspection',evidence=['research/v5/expansion/legacy_systems_startup/resources.json','research/v5/expansion/legacy_systems_startup/provisioning.json']),
 'external_review':dict(state='not-run',evidence=[]),'independent_learner':dict(state='not-applicable-source-access',evidence=[]),'physical_observation':dict(state='not-applicable-source-access',evidence=[])},evidence_limits=['Matching version and inspected body are not a reproduced build or proof of source-to-binary equivalence. No whole-loader audit is claimed.']))
pins=[public,analysis,receipt,check,*canonical,'research/v5/expansion/legacy_systems_startup/resources.json','research/v5/expansion/legacy_systems_startup/provisioning.json']
dump(support,dict(schema_version=1,required=True,scope='30 ordinary/30 changed T10 criteria, LSYS-EXEC-T10 and precise source-access successor',source_sha256={p:sha(ROOT/p) for p in pins},criteria=criteria,updates=updates,source_records=source_records))
dump(folder/'gap_ledger.json',dict(schema_version=1,required=True,gaps=[dict(id='LSYS-EXEC-T10',originating_record='research/v5/expansion/legacy_systems/gap_ledger.json',status='author-implemented-validated; external-dimensions-separate',requirement='30 original T10 criteria and30 changed conditions',support=relative,remaining=['Non-author technical review','Independent ordinary and delayed attempts']),dict(id='LSYS-RUNTIME-HOST-243',originating_record='research/v5/expansion/legacy_systems_elf/gap_ledger.json',status='resolved-for-source-access-and-named-body-correlation',requirement='Authorized matching host glibc source and bounded startup/TLS body inspection',support=relative,limits='No binary reproduction or whole-library equivalence')],historical_note='Original snapshots and accepted evidence are unchanged; this successor supplies dimension-specific current state.'))
statepath=ROOT/'curriculum/v5/expansion/implementation_updates.json';state=read_json(statepath);index={r['id']:r for r in state['updates']}
for row in updates:index[row['id']]=row
state['updates']=list(index.values());dump(statepath,state);print(dict(criteria=len(criteria),author_dispositions=len(updates),learner_claim=False))
