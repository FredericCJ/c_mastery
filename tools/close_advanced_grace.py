"""Bind original epoch/RCU criteria and delayed identities without rewriting them."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_advanced_grace'
from research.v5.expansion.legacy_advanced.author_support import attach
PROJECTS=['PR-T26-02','PR-T26-05'];RECEIPT='research/v5/expansion/legacy_advanced_grace/validation.json';GUEST='research/v5/expansion/legacy_advanced_grace/guest_validation.json';SUPPORT='research/v5/expansion/legacy_advanced_grace/completion_dispositions.json'
ARTIFACTS=['labs/advanced_concurrency_depth/grace/CONTRACTS.md','assessor/advanced_concurrency_depth/grace/ANALYSIS.md','assessor/advanced_concurrency_depth/grace/grace.c','assessor/advanced_concurrency_depth/grace/models.py','assessor/advanced_concurrency_depth/grace/guest/ag_rcu.c','assessor/execution/concurrent.c','assessor/execution/advanced.c','assessor/api_depth/api_depth.c']
CASES={'T26.07.a':['epoch'],'T26.07.b':['versions','callbacks'],'T26.07.c':['pressure','departure'],'T26.08.a':['versions','writers'],'T26.08.b':['versions'],'T26.08.c':['callbacks']}
RATIONALES={'T26.07.a':'Actual retained epoch policy, distinct reachable/injected ledgers, bounded reachability, nested outermost exit, finite registration and section identities.','T26.07.b':'Separate publication/reader/reference/reuse obligations, checked post-section borrow rejection and actual registry-owned capture acquired before exit.','T26.07.c':'Sixteen actual retirements and pool exhaustion, normalized same-payload hazard/epoch comparison, actual join before successor cleanup; paused ownership remains protected.','T26.08.a':'Immutable old/new versions, collection after old reader exits despite a fresh reader, actual two-writer mutex adapter and unowned/competing writer rejection.','T26.08.b':'Actual four-reader CMX comparison and inspected Linux rules, then original hard-interrupt-reader module on a pinned disposable two-struct lr_cpu Linux guest with callback drain and harmless negative control.','T26.08.c':'Actual struct api_registry composition with one retained version reference per successful capture, snapshot removal and reentrant destruction release orders, failure rollback and final collection.'}
REMAINING=['T26.09/.10 kernel/LKMM/guest ordinary and changed conditions remain required (six original criterion instances).','Final published C17/C23 source, external review, independent delayed learner and physical observations remain distinct.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(project):return ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
def selected(d):return [c for c in d['criteria'] if c['objective'] in CASES]
def checks_for(commands,objective):
 result=[x for x in commands if any(x.endswith('.'+c) for c in CASES[objective]) and '.learner.' not in x and '.mutation.' not in x]+['models']
 if objective=='T26.07.b':result += [x for x in commands if '.cortex-' in x]
 return list(dict.fromkeys(result))
def evidence(commands,objective):
 r=dict(report=RECEIPT,checks=checks_for(commands,objective))
 if objective=='T26.08.b':r['additional_reports']=[dict(report=GUEST,checks=['guest'])]
 return r
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT,GUEST]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/advanced_concurrency_depth/grace').rglob('*') if p.is_file())+['labs/execution/include/cmx.h','labs/execution/include/cmx_advanced.h','labs/execution/README.md','labs/execution/targets/README.md','labs/execution/src/support.c','labs/api_depth/include/api_depth.h','labs/api_depth/CONTRACTS.md']
 for project in PROJECTS:
  d=read(canon(project));snapshot=HERE/(project+'.before-grace.json')
  if not snapshot.exists():snapshot.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T26',key='advanced_grace_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/execution/README.md','labs/execution/targets/README.md','labs/api_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_advanced_grace/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in selected(d)],scope='Original T26.07/.08 ordinary and delayed mechanisms; serialized user model and pinned Linux guest remain separate. No independent review, learner or physical claim.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS two additive epoch/RCU attachments; original tasks unchanged')
def package():
 reports=[read(HERE/n) for n in ['validation.json','guest_validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_advanced_grace.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py'};files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  for name,h in data.get('external_input_sha256',{}).items():assert sha(pathlib.Path(name))==h,('stale external',name)
  files|={ROOT/p for p in data['source_sha256']}
 report,guest,exports=reports;commands=[c['id'] for c in report['commands']];rows=[]
 for project in PROJECTS:
  d=read(canon(project));before=read(HERE/(project+'.before-grace.json'));assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
  for c in selected(d):
   v=c['variants'][0]
   for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=report['limits']+guest['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Twelve original criterion instances and twelve exact delayed identities; source-bound author evidence only.',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,GUEST,'research/v5/expansion/legacy_advanced_grace/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=12,delayed=12,parents=0,commands=len(report['commands']),guest_commands=len(guest['commands']),export_commands=len(exports['commands']))));print('PASS twenty-four exact epoch/RCU author dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
