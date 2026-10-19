"""Pure additive T55 attachments; exact original objects and variants retained."""
import argparse,copy,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent;sys.path.insert(0,str(HERE))
from criterion_notes import NOTES
PROJECTS=[f'PR-T55-{i:02}' for i in range(1,11)]
LAB=ROOT/'labs/concurrency_followthrough';REF=ROOT/'assessor/concurrency_followthrough'
SUPPORT='research/v5/expansion/legacy_concurrency_followthrough/support.json';CAMPAIGN='reviews/v5/expansion/legacy_concurrency_followthrough/campaign.json';CONTRACT='labs/concurrency_followthrough/CONTRACTS.md'
OPEN=['Independent external review not performed.','Original minimum-seven-day independent learner variants not administered.','Physical target and hardware-cache/progress observations not performed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def write(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8',newline='\n')
def records():return [json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(p+'.json')).read_text()) for p in PROJECTS]
def public_paths():
 paths={p for p in LAB.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
 paths|={ROOT/p for p in json.loads((LAB/'providers.json').read_text()).values()}
 paths|={ROOT/p for p in ('labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/README.md','labs/api_depth/include/api_depth.h','labs/api_depth/CONTRACTS.md','labs/advanced_concurrency_depth/reclamation/reclamation.h','labs/advanced_concurrency_depth/reclamation/reservation.h','labs/advanced_concurrency_depth/reclamation/tests.c','labs/advanced_concurrency_depth/reclamation/reservation_tests.c','labs/advanced_concurrency_depth/reclamation/CONTRACTS.md','labs/advanced_concurrency_depth/grace/grace.h','labs/advanced_concurrency_depth/grace/tests.c','labs/advanced_concurrency_depth/grace/CONTRACTS.md','curriculum/modules/T55.md')}
 paths|={ROOT/'fixtures/os_v3'/(p+'.json') for p in PROJECTS}|{ROOT/'curriculum/projects'/(p+'.md') for p in PROJECTS}
 return sorted(p.relative_to(ROOT).as_posix() for p in paths)
def attach(record):
 if record['project'] not in PROJECTS:return record
 for c in record['criteria']:c['successor_support']={'path':SUPPORT,'id':c['id']}
 extra=record.setdefault('additional_successor_support',[]);extra[:]=[x for x in extra if x.get('path')!=SUPPORT];extra.append(dict(path=SUPPORT,criterion_ids=[c['id'] for c in record['criteria']],scope='Exact original and V1 construction, models, native instrumentation, finite histories and measured comparisons; independent/physical gates retained.'));extra.sort(key=lambda x:x['path'])
 if CONTRACT not in record['shared_contracts']:record['shared_contracts'].append(CONTRACT)
 for p in public_paths():
  if p not in record['supplied_artifacts']:record['supplied_artifacts'].append(p)
 return record
def prepare():
 rows=[];analysis=['# Exact T55 private assessor derivations','','Preserve evidence kinds: finite model, executed native mechanism, measured host distribution and selected target link are distinct.','']
 for r in records():
  lines=['# '+r['project']+' required followthrough','','Read the original project/module/fixture and `../CONTRACTS.md`. Bring the accepted producer implementation and state the exact dependency manifest before running.','']
  for c in r['criteria']:
   teaching,base,changed,selectors=NOTES[c['objective'].removeprefix('T55.')]
   lines+=['## '+c['objective'],'',teaching,'','Original task: '+c['task'],'','Changed condition: '+c['variants'][0]['task'],'','Deliver the initial prediction, ownership/publication/lifetime argument, completed mechanism or explicit premise counterexample, raw source-bound checks and a changed-condition defense. Keep original rubric levels and at least-seven-day independent attempt separate from these author demonstrations.','']
   analysis+=['## '+c['id'],'',base,'','Changed condition: '+changed,'','Exact evidence selectors: `'+json.dumps(selectors,sort_keys=True)+'`.','']
   rows.append(dict(id=c['id'],objective=c['objective'],project=r['project'],original_task=c['task'],original_expected=c['verification']['expected'],changed_conditions=copy.deepcopy(c['variants']),teaching='labs/concurrency_followthrough/units/'+r['project']+'.md',selectors=selectors))
  dest=LAB/'units'/(r['project']+'.md');dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text('\n'.join(lines)+'\n',encoding='utf-8')
 assert len(rows)==len(NOTES)==30;write(LAB/'criteria.json',dict(schema_version=1,criteria=rows));(REF/'ANALYSIS.md').write_text('\n'.join(analysis)+'\n',encoding='utf-8');print('Prepared30 exact criteria and30 original V1 variants.')
def freeze():
 sys.path[:0]=[str(ROOT),str(ROOT/'tools')];from check_concurrency_followthrough import validate_campaign
 campaign=json.loads((ROOT/CAMPAIGN).read_text());validate_campaign(campaign);updates=[];canonical=[]
 for r in records():
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(r['project']+'.json');write(path,attach(r));canonical.append(path)
  for c in r['criteria']:
   teaching,base,changed,selectors=NOTES[c['objective'].removeprefix('T55.')]
   row=dict(id=c['id'],project=r['project'],original_objective=c['objective'],original_source=path.relative_to(ROOT).as_posix(),closure_rationale=base,selectors=selectors,variant_ids=[v['id'] for v in c['variants']],dimensions={'teaching':{'state':'complete-exact-original-and-changed-condition-contracts','evidence':[CONTRACT,'labs/concurrency_followthrough/criteria.json','labs/concurrency_followthrough/units/'+r['project']+'.md']},'implementation':{'state':'implemented-or-derived-exact-native-model-and-provider-mechanisms','evidence':['assessor/concurrency_followthrough/ANALYSIS.md','assessor/concurrency_followthrough/core/core.c','assessor/concurrency_followthrough/core/epoch.c','assessor/concurrency_followthrough/history/history.c','assessor/concurrency_followthrough/history/oracle.py','assessor/concurrency_followthrough/measure/measure.c','assessor/concurrency_followthrough/models.py']},'author_validation':{'state':'passed-source-bound-mechanisms-independent-finite-oracle-and-native-measurements','evidence':[CAMPAIGN]},'external_review':{'state':'not-run','evidence':[]},'independent_learner':{'state':'not-assessed','evidence':[]},'physical_observation':{'state':'not-run','evidence':[]}},external_blockers=OPEN,required_environment='C17 GCC/Clang, POSIX host verification, explicit frozen original providers; M4 source/link audits do not require a target runtime archive.',evidence_limits=['Finite40000-item multiplicity and67500-case histories are not all-history, weak-memory or unbounded progress proofs.','CfEpoch is an acquire/release serialized try adapter; a stopped gate owner can prevent successful service.','CfPool prepublication cancellation is owner-only; concurrent AR cancellation requires the distinct producer-ack protocol.','Measurements distinguish body-read workload from eight-byte queue records, conservative AP admission, mutex policy, metadata and shared-host uncertainty.','Native ASan/UBSan and TSan evidence is excluded from timing summaries.','Selected M4 symbol closure is not target execution, physical observation or ISR authorization.','Author work does not administer independent original delayed assessment.'])
   updates.append(row)
   for v in c['variants']:
    d=copy.deepcopy(row);d.update(id=v['id'],parent_id=c['id'],original_variant_id=v['id'],original_variant_task=v['task'],minimum_days=v['minimum_days'],closure_rationale=changed,variant_ids=[v['id']],variant_readiness='Exact original changed condition implemented or explicitly derived; independent original delay remains unobserved.');updates.append(d)
 pins={**campaign['source_sha256'],CAMPAIGN:sha(ROOT/CAMPAIGN),**{p.relative_to(ROOT).as_posix():sha(p) for p in canonical}}
 write(ROOT/SUPPORT,dict(schema_version=1,required=True,updates=updates,source_sha256=pins,source_records=[dict(id='R-T55-CONCURRENCY-FOLLOWTHROUGH',path='research/v5/expansion/legacy_concurrency_followthrough/resources.json',passages=['Exact original criteria, inspected producer source and preserved primary-source registrations, distinct mechanism and measured evidence'])],campaigns=[dict(producer='tools/run_concurrency_followthrough.py',report=CAMPAIGN)],remaining_author_work=[],remaining_external_work=OPEN,scope='All30 original T55 criteria and30 exact V1 variants; no independent mastery or physical promotion.'));print(json.dumps(dict(criteria=30,updates=len(updates))))
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');a=p.parse_args();prepare() if a.prepare else freeze()
