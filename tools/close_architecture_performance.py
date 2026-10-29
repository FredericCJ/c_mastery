"""Bind the exact final T37.10 criterion/changed task pairs to current evidence."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_performance'
from research.v5.expansion.legacy_architecture_performance.author_support import attach
PROJECT='PR-T37-08';RECEIPT='research/v5/expansion/legacy_architecture_performance/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_performance/completion_dispositions.json'
ARTIFACTS=['labs/architecture_performance/CONTRACTS.md','assessor/architecture_performance/ANALYSIS.md','assessor/architecture_performance/performance.c','assessor/architecture_performance/oracle.py','labs/architecture_performance/tests.c','labs/architecture_performance/measure.c','assessor/architecture_performance/analyze.py','assessor/architecture_performance/drift.py','assessor/architecture_performance/generate.py','assessor/algorithms/x86_64.S']
RATIONALES={'T37.10.a':'Checked exact rational struct lr_cpu-time/Amdahl with communication-cost break-even and independent Fraction oracle; synthetic original/changed values are separate from native measurements.','T37.10.b':'Actual reused checked C and assembly providers have equivalent failure/admission contracts; six profiles each retain248 native samples across separate size/preparation interventions,31 samples/condition and no-work rejection.','T37.10.c':'All raw timings/overhead and work counts retained; independent median/range and paired blocked analysis with original differential-drift counterexample, explicit migration/frequency/virtualization limits and no speed threshold.'}
REMAINING=['No original PR-T37-08 author implementation task remains; learner/delayed assessment and external review remain unperformed.','Physical MCU timing/energy/WCET and exact final published ISO source evidence remain separate; native author measurements do not close those dimensions.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.10.a':['.equation','.mutation.UNITS','.mutation.COMM'],'T37.10.b':['generate','.equal','.measure','.analysis','.no_work','.mutation.TRUST','.mutation.SIGNED'],'T37.10.c':['drift','.stats','.measure','.analysis','.mutation.OUTLIER','cpu','kernel','libc']};return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def all_rows(r):
 rows=[];commands=[q['id'] for q in r['commands']]
 for c in read(canon())['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-performance.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_performance').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/algorithm_abi/bound.h','labs/algorithm_abi/CONTRACTS.md','labs/algorithms/include/algo.h','labs/algorithms/src/x86_64.S']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0],'labs/algorithm_abi/CONTRACTS.md'],source_register='research/v5/expansion/legacy_architecture_performance/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Three exact ordinary/changed dimensional, equivalent-contract native experiment and distribution/drift mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive performance attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_performance.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith(('attempt','root_'))}
 for d in reports:
  assert d['status']=='pass' and d['commands'] and all(q['passed'] for q in d['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in d[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in d['source_sha256']}
 d=read(canon());old=read(HERE/(PROJECT+'.before-performance.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;rows=all_rows(r);assert len(rows)==6
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Three original T37.10 criterion/V1 pairs',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_performance/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=3,delayed=3,commands=len(r['commands']),export_commands=len(e['commands']),raw_native_samples=1488)));print('PASS6 exact performance dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
