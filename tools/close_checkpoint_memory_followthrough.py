"""Bind exact retained P1 tasks to executed successor mechanisms, preserving history."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_checkpoint_memory.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_checkpoint_memory';PROJECTS=['PR-P1-01','PR-P1-02'];SUPPORT='research/v5/expansion/legacy_checkpoint_memory/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_checkpoint_memory/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'T14.01.c':['gcc-debug:ordinary','clang-asan:transfer'],'T14.05.c':['gcc-debug:ordinary','gcc-debug:core-mutant6','clang-asan:transfer-2'],'T16.02.a':['gcc-debug:ordinary','gcc-debug:core-mutant5','clang:target-O3'],'T18.04.b':['gcc-debug:ordinary','gcc-debug:core-mutant7','clang-asan:transfer'],'T18.04.c':['gcc-debug:core-mutant8','clang-asan:transfer'],'T13.01.c':['gcc:target-O0','gcc:target-O3','clang:target-O0','clang:target-O3','gcc:effects1','clang:effects2','lto-dependencies'],'T16.14.b':['gcc-debug:ledger','clang-asan:ledger','clang-asan:transfer','gcc-debug:core'],'T16.14.c':['gcc:measure','clang:measure'],'T21.04.b':['gcc-debug:transfer-1','clang-asan:transfer','gcc-debug:adapters','clang-asan:adapters']}
RATIONALE={'T14.01.c':'Actual declared byte/int roundtrip, typed pool and allocated typed records with separate deallocation.','T14.05.c':'Literal FIFO trace and staged rollback; independent6561 eight-action histories; linked-node failure cuts retain accepted prefix.','T16.02.a':'Exact32/header3/alignment8 arithmetic and8/16/refused32 trace; exact-fit/one-short/overflow/insufficient base guarantee preserve state; actual target execution.','T18.04.b':'Distinct queue/callback pins, cancellation retains pending lease, matching quiescent completion and stale/duplicate rejection.','T18.04.c':'Finite generation retirement rather than wrap, stale handle rejection and real allocated backing relocation with pin/failure controls.','T13.01.c':'Four actual strict M4 images and five-object audits, comparator/logger heap/wait controls, actual native LTO/stack-protection/resolved-provider inspection.','T16.14.b':'Independent live extent/alignment/nonoverlap/content ledger for3init/2growth/4node cuts, exact-fit/one-short and unchanged four-input half-capacity transfer.','T16.14.c':'744 actual paired arena/pool samples, independent byte/checksum accounting, three fixed-capacity distributions, instrumentation on/off and honest unequal admission/memory scope.','T21.04.b':'Three-stage construction cleanup, async unregister state and actual unchanged later struct ow_owner import with measured control bytes/callbacks; no invented concurrency guarantee.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def card(project):return 'curriculum/v5/expansion/legacy_assessments/'+project+'.json'
def original(project):return json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+card(project)],cwd=ROOT,text=True,encoding='utf8'))
def historical_equal(now,old):
 def criteria(d):return [{k:v for k,v in c.items() if k!='successor_support'} for c in d['criteria']]
 return criteria(now)==criteria(old) and now['baseline_binding']==old['baseline_binding']
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/checkpoint_memory_followthrough').iterdir()) if p.is_file()]
 public+=['labs/checkpoint_depth/include/checkpoint_memory.h','labs/checkpoint_depth/src/checkpoint_memory.c','labs/checkpoint_depth/tests/memory.c','labs/checkpoint_depth/transfer/memory.h','labs/checkpoint_depth/transfer/memory_tests.c','labs/checkpoint_depth/transfer/memory_stub.c']
 mapping=[]
 for project in PROJECTS:
  d=read(ROOT/card(project));old=original(project);assert historical_equal(d,old)
  put(HERE/('attachment_'+project+'.json'),dict(project=project,domain=d['domain'],key='memory_followthrough_successor',supplied_artifacts=public,shared_contracts=['labs/checkpoint_memory_followthrough/CONTRACTS.md'],source_register='research/v5/expansion/legacy_checkpoint_memory/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact original P1 ordinary/delayed tasks with cross-target/effects/resource gap correction; actual learner/physical evidence separate'))
  x=attach(d);assert x['criteria']==d['criteria'] and historical_equal(x,old) and attach(copy.deepcopy(x))==x;put(ROOT/card(project),x)
  for c in d['criteria']:mapping.append(dict(project=project,criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/checkpoint_memory_followthrough/CONTRACTS.md',learner_artifacts=['own core/allocated/queue providers','source-bound effects and ownership ledger','independent boundaries/fault oracle','raw paired timing and target object dossier'],assessment='assessor/checkpoint_memory_followthrough/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective']],source_register='research/v5/expansion/legacy_checkpoint_memory/resources.json'))
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='P2-P6/CAP and other recursive inventory obligations remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md',HERE/'source_access.json']},limitations=['N1570 C11 draft is not published C17.','Native LTO inspection, emulator execution and physical target observations are separate.']))
 print('PASS18 retained tasks; additive idempotent attachment')
def package():
 from check_checkpoint_memory_followthrough import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_checkpoint_memory_package.py',ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py'}
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
    artifacts=['labs/checkpoint_memory_followthrough/CONTRACTS.md','assessor/checkpoint_memory_followthrough/ANALYSIS.md','assessor/checkpoint_memory_followthrough/DELAYED.md','research/v5/expansion/legacy_checkpoint_memory/objective_mapping.json']
    dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=SELECTORS[c['objective']],analysis='assessor/checkpoint_memory_followthrough/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[c['objective']],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='P1 nine ordinary and nine unchanged delayed tasks, including correction of prior incomplete effects/target and measurement scope',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['P2-P6/CAP and other recursive inventory authoring remain required.','External review, independent/delayed learner and physical observations remain separate.'],summary=dict(criteria=9,delayed=9,commands=len(report['commands']),assertions=len(report['checks']))));print('PASS18 exact P1 dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
