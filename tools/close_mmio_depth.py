"""Canonical PR-T28-01 attachment and eighteen exact author dispositions."""
from pathlib import Path
import argparse,hashlib,json,subprocess,sys,copy
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
HERE=ROOT/'research/v5/expansion/legacy_advanced_mmio'
SUPPORT='research/v5/expansion/legacy_advanced_mmio/completion_dispositions.json'
PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-01.json'
REVIEW='reviews/v5/expansion/legacy_advanced_mmio/'
REPORTS=[REVIEW+s for s in ['current_validation.json','exports.json','support_validation.json']]
from research.v5.expansion.legacy_advanced.author_support import attach
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/peripheral_depth/mmio').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]+['labs/firmware_io/include/firmware_io.h','labs/firmware_io/CONTRACTS.md','labs/firmware_io/README.md']
 put(HERE/'attachment_PR-T28-01.json',dict(project=d['project'],domain='T28',key='mmio_gpio_successor',supplied_artifacts=public,shared_contracts=['labs/peripheral_depth/mmio/CONTRACTS.md','labs/firmware_io/CONTRACTS.md'],source_register='research/v5/expansion/legacy_advanced_mmio/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Only T28.01/.02/.04 nine ordinary+nine delayed; original FW-03 reference composition; other T28 tasks remain required'))
 x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/PROJECT,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/peripheral_depth/mmio/CONTRACTS.md',analysis='assessor/peripheral_depth/mmio/ANALYSIS.md',source_register='research/v5/expansion/legacy_advanced_mmio/resources.json') for c in d['criteria']],remaining='T28.03/.05–.12 required; physical/source/learner/reviewer evidence separate'))
 gate=read(HERE/'research_gate.json');gate['research_input_sha256']={p:sha(ROOT/p) for p in gate['research_input_sha256']};put(HERE/'research_gate.json',gate)
 print('PASS original nine criterion objects preserved and additive attachment idempotent')
RATIONALE={
 '01.a':'Revision-specific masks and width rejection before effects; actual layout assertions and source-specific register table.',
 '01.b':'W1C direct-mask versus stale RMW, consuming binary-zero/255 reads and one-shot read-clear snapshot with distinct compiled controls.',
 '01.c':'Readiness/publication state machine, bounded polls and an implemented interrupt/deadline adapter with pre-arm readiness recheck, spurious events, final-tick readiness and no-IRQ deadline.',
 '02.a':'Explicit32-bit volatile accesses, offset assertions, four actual M4 object/image audits, three actual PPC64 big-endian model profiles, compiled wrong-width rejection.',
 '02.b':'Criterion-specific reasoning separates compiler access, bus transaction, ordering and ownership; explicit data-race counterexample and platform/cache transfer obligations. No executed race-value oracle or unsupported physical ordering guarantee.',
 '02.c':'Actual model trace ODR lost update6 versus BSRR14; conflicting whole-register writer counterexample and original single-write provider composition.',
 '04.a':'Separate pin configuration/routing ledger with inspected package and board rows, explicit unknown assembly, and16-state original open-drain/floating/contention model.',
 '04.b':'One logged multibit write, boundary/overlap policy and actual FW-03 provider across32 pin/level cases; cross-port intermediate-state analysis.',
 '04.c':'Consecutive-sample and duration-based filters, irregular/wrapping timestamps, gap reset, transactional event-counter exhaustion, independent suffix oracle and compiled wrong-duration control.'}
def package():
 from check_mmio_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_mmio_package.py',ROOT/PROJECT,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for p in REPORTS:
  data=read(ROOT/p);assert data['passed'],p;files.add(ROOT/p)
  for name,h in data.get('input_sha256',{}).items():assert sha(ROOT/name)==h,(p,name);files.add(ROOT/name)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 d=read(ROOT/PROJECT);assert d['criteria']==original()['criteria'];rows=[]
 for c in d['criteria']:
  key=c['objective'][4:];v=c['variants'][0]
  for delayed in [False,True]:
   artifacts=['labs/peripheral_depth/mmio/CONTRACTS.md','assessor/peripheral_depth/mmio/ANALYSIS.md','research/v5/expansion/legacy_advanced_mmio/objective_mapping.json']
   dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   labels=[] if key=='02.b' else [r['label'] for r in report['checks'] if ('execute 0' in r['label'] or 'independent oracle' in r['label'] or (key=='02.a' and any(s in r['label'] for s in ['target disassembly','image closure','big-endian'])) or (key in ['02.c','04.b'] and 'composition execute' in r['label']))]
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/peripheral_depth/mmio/ANALYSIS.md#'+key,kind='reasoning-only semantic counterexample' if key=='02.b' else 'compiled mechanism plus criterion-specific reasoning'),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Published C17/C23 access gaps remain; N1570 is the inspected older draft.','Author execution is not external review, independent learner or delayed evidence.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T28-01 nine ordinary and nine unchanged delayed tasks only',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T28.03 andT28.05–.12 remain required authoring/implementation work.','Physical board observations, published ISO access, independent learners and review remain separate.'],summary=dict(criteria=9,delayed=9,commands=len(report['checks']),export_commands=len(read(ROOT/REPORTS[1])['commands']))))
 print('PASS eighteen exact MMIO/GPIO author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
