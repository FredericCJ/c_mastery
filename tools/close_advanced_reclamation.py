"""Bind twelve original queue/protection criteria and exact delayed variants."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_advanced_reclamation'
from research.v5.expansion.legacy_advanced.author_support import attach
PROJECTS=['PR-T26-01','PR-T26-02','PR-T26-05'];RECEIPT='research/v5/expansion/legacy_advanced_reclamation/validation.json';SUPPORT='research/v5/expansion/legacy_advanced_reclamation/completion_dispositions.json'
ARTIFACTS=['labs/advanced_concurrency_depth/reclamation/CONTRACTS.md','assessor/advanced_concurrency_depth/reclamation/ANALYSIS.md','assessor/advanced_concurrency_depth/reclamation/reclamation.c','assessor/advanced_concurrency_depth/reclamation/reservation.c','assessor/advanced_concurrency_depth/reclamation/models.py','assessor/advanced_concurrency_depth/reclamation/history_oracle.py','assessor/execution/concurrent.c','assessor/execution/advanced.c']
CASES={'T26.04.a':['publication','retention','histories'],'T26.04.b':['publication','reservation','histories'],'T26.04.c':['stress','histories'],'T26.06.a':['validation','protection','collector','histories'],'T26.06.b':['retention','generation','stress'],'T26.06.c':['departure','registration','protection']}
RATIONALES={'T26.04.a':'Complete bounded queue and actual paused publisher/consumer helping, real invocation histories and independent FIFO witnesses; retained hazards produce explicit failed admission.','T26.04.b':'Actual retained reservation prefix compared to link publication, implemented cancellation request/owner acknowledgement/tombstone, stale ticket and exhausted ticket controls; stalled owner still blocks.','T26.04.c':'Separate second-producer/consumer SPSC countermodels and complete unique-allocation/link/head/protection repair, real2+2/4+4 stress and FIFO history witnesses.','T26.06.a':'Full protection/validation/retirement/scan, forced removal and stale collector incarnation, SC proof/countermodel and implemented single relaxed candidate-load variant after the full-state CAS proof; original regressions run on both.','T26.06.b':'Exact four/eight-participant same32-node ledgers with6/14 held and25/17 queue admission, one-node recovery and63/127 finite generation exhaustion; scan and retention bounds distinguished.','T26.06.c':'Actual join before transferred cleanup, paused validated reader retained, implemented registration generation and terminal saturation, stale lease cannot clear replacement hazards; no timeout-as-termination assumption.'}
REMAINING=['T26.07/.08 epoch and RCU ordinary/changed conditions remain required (twelve original criterion instances).','T26.09/.10 kernel/LKMM/guest ordinary/changed conditions remain required (six original criterion instances).','Final published C17/C23 source, external review, independent delayed learner and physical observations remain distinct.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(project):return ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
def selected(d):return [c for c in d['criteria'] if c['objective'] in CASES]
def checks_for(commands,objective):
 result=[x for x in commands if any(x.endswith('.'+c) for c in CASES[objective]) and '.learner.' not in x and '.mutation.' not in x]+['models']
 if objective.startswith('T26.04'):result+=['spsc.models']+[x for x in commands if x.endswith('.history.oracle')]
 if objective=='T26.06.a':result += [x for x in commands if '.relaxed.' in x]+[x for x in commands if '.cortex-' in x]
 return list(dict.fromkeys(result))
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/advanced_concurrency_depth/reclamation').iterdir() if p.is_file())+['labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/README.md','labs/advanced_concurrency_depth/history/history.h','labs/advanced_concurrency_depth/history/learner.c','labs/advanced_concurrency_depth/history/CONTRACTS.md','labs/advanced_concurrency_depth/spsc/CONTRACTS.md']
 for project in PROJECTS:
  d=read(canon(project));snapshot=HERE/(project+'.before-reclamation.json')
  if not snapshot.exists():snapshot.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T26',key='advanced_reclamation_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/execution/README.md','labs/advanced_concurrency_depth/history/CONTRACTS.md','labs/advanced_concurrency_depth/spsc/CONTRACTS.md'],source_register='research/v5/expansion/legacy_advanced_reclamation/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in selected(d)],scope='Only original T26.04/.06 ordinary and delayed mechanisms; no independent review, unrestricted participant departures or universal progress claim.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS three additive queue/protection attachments; original tasks unchanged')
def package():
 report=read(HERE/'validation.json');exports=read(HERE/'exports.json');files={pathlib.Path(__file__),ROOT/'tools/check_advanced_reclamation.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py'};files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in [report,exports]:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  files|={ROOT/p for p in data['source_sha256']}
 commands=[c['id'] for c in report['commands']];rows=[]
 for project in PROJECTS:
  d=read(canon(project));before=read(HERE/(project+'.before-reclamation.json'));assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
  for c in selected(d):
   v=c['variants'][0]
   for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=dict(report=RECEIPT,checks=checks_for(commands,c['objective'])),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=report['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Twelve original criterion instances and twelve original delayed IDs; source-bound author evidence only.',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_advanced_reclamation/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=12,delayed=12,parents=0,commands=len(report['commands']),export_commands=len(exports['commands']))));print('PASS twenty-four exact queue/protection author dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
