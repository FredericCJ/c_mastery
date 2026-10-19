"""Pure additive exact T56 support; retained PR-T56-07 belongs to its provider."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_services_followthrough.criterion_notes import NOTES
PROJECTS=[f'PR-T56-{n:02}' for n in range(1,13) if n!=7]
LAB=ROOT/'labs/services_followthrough';REF=ROOT/'assessor/services_followthrough'
SUPPORT='research/v5/expansion/legacy_services_followthrough/support.json';CAMPAIGN='reviews/v5/expansion/legacy_services_followthrough/campaign.json';CONTRACT='labs/services_followthrough/CONTRACTS.md'
OPEN=['Independent external review not performed.','Original minimum-seven-day independent learner variants not administered.','Physical H745 progress, synchronization, overload, restart and timing observations not run.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 paths={p for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
 for name in ('labs/kernel_smp/include','labs/kernel_smp/platform','labs/kernel_smp/learner','labs/service_depth/adapters/freestanding','labs/runtime/include','labs/storage/include'):
  paths.update(p for p in (ROOT/name).rglob('*') if p.is_file() and '__pycache__' not in p.parts)
 paths.update(ROOT/p for p in ('labs/kernel_smp/tools/guest.py','labs/kernel_smp/tests/spawn_user.c','labs/service_depth/adapters/kernel_abi.h','labs/service_depth/adapters/receipt.h','labs/service_depth/adapters/receipt.c','labs/service_depth/adapters/load.inc','labs/service_depth/adapters/services.inc','labs/runtime_followthrough/verifier/verifier.h','labs/runtime_followthrough/verifier/learner.c','labs/runtime_followthrough/execution/execution.h','labs/runtime_followthrough/execution/learner.c','labs/amp_followthrough/core/amp.h','labs/amp_followthrough/core/amp.c','labs/amp_followthrough/target/start.S','labs/amp_followthrough/BOARD_CAPTURE.md','labs/amp_followthrough/capture.py','labs/amp_followthrough/capture_template.json','labs/mcu_followthrough/baseline/support.c','curriculum/modules/T56.md'))
 paths.update(ROOT/'fixtures/os_v3'/(p+'.json') for p in PROJECTS);paths.update(ROOT/'curriculum/projects'/(p+'.md') for p in PROJECTS)
 paths.update(ROOT/p for p in ('labs/runtime/src/cvm.c','labs/storage/src/store.c'))
 return sorted(p.relative_to(ROOT).as_posix() for p in paths)
def attach(record):
 if record['project'] not in PROJECTS:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 extra=record.setdefault('additional_successor_support',[]);extra[:]=[x for x in extra if x.get('path')!=SUPPORT];extra.append(dict(path=SUPPORT,criterion_ids=[c['id'] for c in record['criteria']],scope='Exact original/V1 service mechanisms, actual native/SMP/image checks, explicit external physical and independent gates.'));extra.sort(key=lambda x:x['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for p in public_paths():
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def prepare():
 rows=[];analysis=['# Exact T56 private assessor derivations','','The original contracts and narrower historical receipts remain valid for their stated scope. Each cited guest case is an actual mechanism; physical and independent observations are separate.','']
 for r in records():
  lines=['# '+r['project']+' exact followthrough','','Read the original project/module/fixture and `../CONTRACTS.md`. Complete the public scaffold and select prerequisite sources explicitly. Predict ownership, publication and failure before opening assessor material.','']
  for c in r['criteria']:
   teaching,base,changed,cases=NOTES[c['objective'].removeprefix('T56.')]
   lines+=['## '+c['objective'],'',teaching,'','Original task: '+c['task'],'','Changed condition: '+c['variants'][0]['task'],'','Required evidence: your initial prediction, complete ownership/identity ledger, mechanism implementation or explicit premise counterexample, exact source/commands/raw results and changed-condition explanation. Use guest cases '+', '.join(map(str,cases))+' as executable practice. A passing author demonstration does not administer the original independent delayed attempt.','']
   analysis+=['## '+c['id'],'',base,'','Changed condition: '+changed,'','Exact guest selectors: '+', '.join(map(str,cases))+'.','']
   rows.append(dict(id=c['id'],objective=c['objective'],project=r['project'],original_task=c['task'],original_expected=c['verification']['expected'],changed_conditions=copy.deepcopy(c['variants']),teaching='labs/services_followthrough/units/'+r['project']+'.md',guest_cases=cases))
  dest=LAB/'units'/(r['project']+'.md');dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 assert len(rows)==len(NOTES)==33;write(LAB/'criteria.json',dict(schema_version=1,criteria=rows));(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8')
 print('Prepared33 exact criteria and33 original V1 variants; PR-T56-07 retained.')
def freeze():
 sys.path[:0]=[str(ROOT),str(ROOT/'tools')];from check_services_followthrough import validate_campaign
 campaign=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(campaign);updates=[];canonical=[]
 implementation=['assessor/services_followthrough/ANALYSIS.md','assessor/services_followthrough/kernel/process.inc','assessor/services_followthrough/kernel/faults.inc','assessor/services_followthrough/kernel/channels.inc','assessor/services_followthrough/kernel/shared.inc','assessor/services_followthrough/kernel/extra.inc','assessor/services_followthrough/vm/service.c','assessor/services_followthrough/store/service.c','assessor/services_followthrough/workload/workload.c','assessor/services_followthrough/workload/board.c']
 for r in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(r['project']+'.json');write(path,attach(r));canonical.append(path)
  for c in r['criteria']:
   teaching,base,changed,cases=NOTES[c['objective'].removeprefix('T56.')]
   row=dict(id=c['id'],project=r['project'],original_objective=c['objective'],original_source=path.relative_to(ROOT).as_posix(),closure_rationale=base,guest_cases=cases,variant_ids=[v['id'] for v in c['variants']],dimensions={'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/services_followthrough/criteria.json','labs/services_followthrough/units/'+r['project']+'.md']},'implementation':{'state':'implemented-or-derived-exact-service-and-target-mechanisms','evidence':implementation},'author_validation':{'state':'passed-source-bound-native-guest-image-and-export-checks','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run-H745-capture-required','evidence':['labs/services_followthrough/BOARD_CAPTURE.md']}},external_blockers=OPEN,required_environment='C17 GCC/Clang, explicit original freestanding providers, original disposable QEMU x86-64 guest; H745 images audited without board execution.',evidence_limits=['A source-bound actual guest/emulator receipt is not a physical, human-independent or all-interleaving proof.','Before-lock fault arrival and IO observer delay are explicit staged test controls, not host policy changes or hardware cancellation acknowledgments.','Native CLOCK_MONOTONIC time, QEMU TSC counts and unobserved physical clocks remain distinct.','H745 workload images are bounded synthetic-input polling role experiments; retained AMP scheduler/DMA/restart images have separate hashes and scope.','A three-receipt durable window preserves no-reapply safety but does not promise indefinite retrieval of an old result.','The inherited receipt cut campaign uses its original value7 and flush/sector model; new real service restart/reboot cases separately establish value42.','Author work does not administer independent original delayed assessment.'])
   updates.append(row)
   for v in c['variants']:
    d=copy.deepcopy(row);d.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=changed,variant_ids=[v['id']],variant_readiness='Exact changed condition implemented/derived; physical and independent original-delay observations remain separately open.');updates.append(d)
 pins={**campaign['source_sha256'],CAMPAIGN:sha(ROOT/CAMPAIGN),**{p.relative_to(ROOT).as_posix():sha(p) for p in canonical}}
 write(ROOT/SUPPORT,dict(schema_version=1,required=True,updates=updates,source_sha256=pins,source_records=[dict(id='R-T56-SERVICES-FOLLOWTHROUGH',path='research/v5/expansion/legacy_services_followthrough/resources.json',passages=['Exact criteria and source imports, unchanged provider receipts, explicit staged instrumentation and evidence limits'])],campaigns=[dict(producer='tools/run_services_followthrough.py',report=CAMPAIGN)],remaining_author_work=[],remaining_external_work=OPEN,scope='All33 previously remaining original T56 criteria and33 exact V1 variants. PR-T56-07 remains under its frozen producer; no independent mastery or physical promotion.'))
 print(json.dumps(dict(criteria=33,updates=len(updates))))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else freeze()
