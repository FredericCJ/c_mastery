"""Bind only six original SPSC criteria and their unchanged delayed tasks."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_advanced_spsc'
from research.v5.expansion.legacy_advanced.author_support import attach
PROJECTS=['PR-T26-01','PR-T26-04'];RECEIPT='research/v5/expansion/legacy_advanced_spsc/validation.json';SUPPORT='research/v5/expansion/legacy_advanced_spsc/completion_dispositions.json'
ARTIFACTS=['labs/advanced_concurrency_depth/spsc/CONTRACTS.md','assessor/advanced_concurrency_depth/spsc/ANALYSIS.md','assessor/advanced_concurrency_depth/spsc/spsc.c','assessor/advanced_concurrency_depth/spsc/models.py','assessor/execution/concurrent.c']
CASES={'T26.03.a':['interface','edges','retained'],'T26.03.b':['edges'],'T26.03.c':['interface','wrap','circular']}
RATIONALES={'T26.03.a':'Actual retained scalar producer and explicit enum-status multiword adapter, capacity boundaries, same-domain reentry rejection and symbolic two-owner collision; no false MPMC promise.','T26.03.b':'Both ownership edges, forced prepublication/prereuse boundaries,16000 multiword transfers/profile and separate missing-edge relation controls; native undefined executions are not value oracles.','T26.03.c':'1728 fresh reduced states/2944 edges, two-wrap coherent stale-observation model, native near-wrap projection, implemented N-1 variant, actual placement/lock-free queries and separate core/composed target link audits.'}
REMAINING=['LRUN-18: other linearizability/progress/MPMC/identity/hazard/epoch/RCU criteria remain required; this SPSC package does not close the parent.','LRUN-19: exact kernel/LKMM/guest criteria need their separate evidence bindings.','Final published C17/C23 source, independent learner, external review and physical observations remain distinct.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(project):return ROOT/'curriculum/v5/expansion/legacy_assessments'/(project+'.json')
def selected(d):return [c for c in d['criteria'] if c['objective'] in CASES]
def checks_for(commands,objective):
 result=[x for x in commands if any(x.endswith('.'+c) for c in CASES[objective]) and '.learner.' not in x and '.mutation.' not in x]
 result+=['models']
 if objective=='T26.03.c':result += [x for x in commands if any(x.startswith(c+'.cortex-') for c in ['arm-none-eabi-gcc','clang']) or x.startswith('implicit-copy.')]
 return list(dict.fromkeys(result))
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/advanced_concurrency_depth/spsc').iterdir() if p.is_file())+['labs/execution/include/cmx.h','labs/execution/README.md']
 for project in PROJECTS:
  d=read(canon(project));snapshot=HERE/(project+'.before-spsc.json')
  if not snapshot.exists():snapshot.write_bytes(canon(project).read_bytes())
  write(HERE/('attachment_'+project+'.json'),dict(project=project,domain='T26',key='advanced_spsc_successor',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/execution/README.md'],source_register='research/v5/expansion/legacy_advanced_spsc/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in selected(d)],scope='Only original T26.03.a/b/c ordinary and delayed SPSC obligations; author execution, not learner or other advanced-concurrency completion.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(project),after)
 print('PASS two additive SPSC attachments; original criteria and baseline unchanged')
def package():
 report=read(HERE/'validation.json');exports=read(HERE/'exports.json');files={pathlib.Path(__file__),ROOT/'tools/check_advanced_spsc.py',ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 files|={p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt')}
 for data in [report,exports]:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for name,h in data[key].items():assert sha(ROOT/name)==h,('stale',name)
  files|={ROOT/p for p in data['source_sha256']}
 commands=[c['id'] for c in report['commands']];rows=[]
 for project in PROJECTS:
  d=read(canon(project));before=read(HERE/(project+'.before-spsc.json'));assert before['criteria']==d['criteria'] and before['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(project))
  for c in selected(d):
   v=c['variants'][0]
   for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=dict(report=RECEIPT,checks=checks_for(commands,c['objective'])),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=report['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Six exact original SPSC criterion instances and six unchanged delayed variants; other advanced concurrency remains required.',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_advanced_spsc/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,parents=0,commands=len(report['commands']),export_commands=len(exports['commands']))));print('PASS twelve exact SPSC author dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
