"""Bind exact retained P5 tasks to executed successor mechanisms, preserving history."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_checkpoint_linux.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_checkpoint_linux';PROJECTS=['PR-P5-01','PR-P5-02'];SUPPORT='research/v5/expansion/legacy_checkpoint_linux/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_checkpoint_linux/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'T24.08.b': ['gcc-debug:fd7', 'gcc-debug:ordinary', 'clang-asan:core-29', 'clang-asan:core-30'], 'T25.02.b': ['gcc-debug:ordinary', 'clang-tsan:ownership', 'clang-asan:core-28'], 'T25.02.c': ['gcc-debug:ownership', 'clang-tsan:ownership'], 'T18.04.c': ['gcc-debug:ordinary', 'clang-asan:core-23', 'clang-asan:core-24', 'clang-asan:core-31', 'clang-asan:core-32'], 'T24.11.b': ['gcc-debug:ordinary', 'clang-asan:bridge', 'gcc-debug:failures', 'clang-asan:core-25', 'clang-asan:core-26'], 'T25.04.c': ['gcc-debug:ordinary', 'clang-tsan:waiter', 'gcc-debug:waiter-27'], 'T21.06.b': ['gcc-debug:bridge', 'clang-tsan:bridge', 'clang-asan:bridge-1', 'clang-asan:bridge-2', 'clang-asan:bridge-3'], 'T21.06.c': ['gcc-debug:ordinary', 'clang-tsan:report-order', 'clang-asan:bridge-2', 'clang-asan:core-32']}
RATIONALE={'T24.08.b': 'Actual fd7 retirement/reuse and stale-before-access, LT/ET blocked parser resumption and actual two-worker pipeline.', 'T25.02.b': 'Literal pressure admission1,2,3,5/reject4,6, ownership conservation, blocking producer/closure and two-worker handoff.', 'T25.02.c': 'Same64-message copied versus shared mutable workload, explicit locks/owner-local data and different resource contracts.', 'T18.04.c': 'Actual copied inline and128-byte external payload, allocation failure, overwritten input and stale generation rejection.', 'T24.11.b': 'Graceful/abort ownership, deadline retention, acknowledgement versus join, creation rollback/abandon and actual fd reuse with injected late close failures.', 'T25.04.c': 'Predicate wait controls plus deterministic real unlocked competing consumption seam; blocked-producer close and abort dispositions.', 'T21.06.b': 'Original struct f_async/in_shutdown/algo_ring imported into real poll/read callback bridge; synchronized matching quiescence, stale tokens and one release.', 'T21.06.c': 'Two generations, duplicate/stale rejection, explicit ARFARF trace, reporter error17, refused reentry and generation retirement.'}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def card(project):return 'curriculum/v5/expansion/legacy_assessments/'+project+'.json'
def original(project):return json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+card(project)],cwd=ROOT,text=True,encoding='utf8'))
def historical_equal(now,old):
 def criteria(d):return [{k:v for k,v in c.items() if k!='successor_support'} for c in d['criteria']]
 return criteria(now)==criteria(old) and now['baseline_binding']==old['baseline_binding']
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/checkpoint_linux_followthrough').iterdir()) if p.is_file()]
 for directory in ['labs/checkpoint_depth/include','labs/integration/include','labs/foundations/include','labs/algorithms/include']:
  public += [p.relative_to(ROOT).as_posix() for p in sorted((ROOT/directory).rglob('*')) if p.is_file()]
 public += ['labs/checkpoint_depth/'+kind+'/'+name+'.c' for kind in ['src','tests'] for name in (['checkpoint_depth','checkpoint_async','checkpoint_linux','checkpoint_operation'] if kind=='src' else ['async','linux','operation'])]
 mapping=[]
 for project in PROJECTS:
  d=read(ROOT/card(project));old=original(project);assert historical_equal(d,old)
  put(HERE/('attachment_'+project+'.json'),dict(project=project,domain=d['domain'],key='linux_followthrough_successor',supplied_artifacts=public,shared_contracts=['labs/checkpoint_linux_followthrough/CONTRACTS.md'],source_register='research/v5/expansion/legacy_checkpoint_linux/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact original P5 ordinary/delayed tasks with actual readiness/callback/ownership/changed-condition and failure behavior; actual learner/physical evidence separate'))
  x=attach(d);assert x['criteria']==d['criteria'] and historical_equal(x,old) and attach(copy.deepcopy(x))==x;put(ROOT/card(project),x)
  for c in d['criteria']:mapping.append(dict(project=project,criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/checkpoint_linux_followthrough/CONTRACTS.md',learner_artifacts=['own complete P1/P3/operation/thread providers','descriptor/ownership/predicate proof and failure traces','admission and completion ledgers','source-bound local execution dossier'],assessment='assessor/checkpoint_linux_followthrough/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective']],source_register='research/v5/expansion/legacy_checkpoint_linux/resources.json'))
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='CAP and other recursive inventory obligations remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md',HERE/'source_access.json']},limitations=['Active C17 baseline; inspected OS and POSIX contracts do not confer ISO C ownership enforcement.','Controlled injected error/scheduling outcomes are separate from spontaneously observed OS failures.']))
 print('PASS16 retained tasks; additive idempotent attachment')
def package():
 from check_checkpoint_linux_followthrough import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_checkpoint_linux_package.py',ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py'}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 for r in read(HERE/'resources.json')['resources']:
  for f,h in r['inspection']['source_sha256'].items():assert sha(ROOT/f)==h;files.add(ROOT/f)
 rows=[]
 for project in PROJECTS:
  files.add(ROOT/card(project));d=read(ROOT/card(project));assert historical_equal(d,original(project))
  for c in d['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    artifacts=['labs/checkpoint_linux_followthrough/CONTRACTS.md','assessor/checkpoint_linux_followthrough/ANALYSIS.md','assessor/checkpoint_linux_followthrough/DELAYED.md','research/v5/expansion/legacy_checkpoint_linux/objective_mapping.json']
    dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=SELECTORS[c['objective']],analysis='assessor/checkpoint_linux_followthrough/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[c['objective']],remaining_author_work=[],external_blockers=[dict(dimension='external_review',missing='Non-author technical reviewer'),dict(dimension='independent_learner',missing='Independent learner attempt and retained minimum seven-day delayed attempt')],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='P5 eight ordinary and eight unchanged delayed tasks; implemented real local IO/synchronization/failure variants, learner and external observations separate',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['CAP and other recursive inventory authoring remain required.','External review, independent/delayed learner and physical observations remain separate.'],summary=dict(criteria=8,delayed=8,commands=len(report['commands']),assertions=len(report['checks']))));print('PASS16 exact P5 dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
