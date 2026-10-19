"""Attach and bind exact original layers criteria; keep external evidence separate."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_layers';PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-06.json';SUPPORT='research/v5/expansion/legacy_advanced_layers/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_layers/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json','cost_analysis.json']]
RATIONALE={'11.a':'Service framing and lifetime, actual PuDriver/F439 adapters and cancellation cuts0..8 with independent CRC oracle.', '11.b':'Actual effect model with W1C, read-consume, delayed readiness, posted publication and irreversible-prefix quarantine.', '11.c':'Equal bounded original/generated/actual HAL transfers, callback restart, frozen ticks, source-pinned F439/G474 map transfer and linked code/stack costs.'}
SELECTORS={'11.a':['gcc-debug service execute 1','gcc-debug service execute 2','gcc-debug uart execute 0','gcc-debug target execute 0','gcc-debug target execute 1','clang-O3 combined audit link'], '11.b':['gcc-debug posted execute 0','gcc-debug posted execute 1','gcc-debug posted execute 2','gcc-debug posted execute 3'], '11.c':['gcc-debug compare execute 0','gcc-debug compare execute 1','gcc-debug compare execute 2','clang-O3 vendor compare 0','clang-O3 vendor callback 0','clang-O3 vendor frozen_tick 0','clang-O3 G474 transfer 0','clang-O3 G474 transfer 2','clang-O3 cost vendor size']}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/peripheral_depth/layers').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public += [p.relative_to(ROOT).as_posix() for folder in ['labs/peripheral_depth/uart','labs/peripheral_depth/mmio'] for p in sorted((ROOT/folder).rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public += ['labs/serial_depth/include/serial_depth.h','labs/serial_depth/src/serial_depth.c','labs/firmware_io/include/firmware_io.h']
 put(HERE/'attachment_PR-T28-06.json',dict(project=d['project'],domain='T28',key='layered_service_successor',supplied_artifacts=public,shared_contracts=['labs/peripheral_depth/layers/CONTRACTS.md','labs/peripheral_depth/uart/CONTRACTS.md','labs/peripheral_depth/mmio/CONTRACTS.md'],source_register='research/v5/expansion/legacy_advanced_layers/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria'] if c['objective'].startswith('T28.11')],scope='Three T28.11 ordinary and three unchanged delayed tasks; actual PuDriver/PmModel/HAL providers; T28.12 and physical/external work separate'))
 x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/PROJECT,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/peripheral_depth/layers/CONTRACTS.md',learner_artifacts=['service.c','uart_adapter.c','posted.c','compare.c','target.c','criterion-indexed lifetime/visibility/maintenance dossier'],assessment='assessor/peripheral_depth/layers/ANALYSIS.md#'+c['objective'][4:],source_register='research/v5/expansion/legacy_advanced_layers/resources.json') for c in d['criteria'] if c['objective'].startswith('T28.11')],remaining='T28.12 and T29 remain required; physical/source/external dimensions separate'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',scope='Exact T28 UART/HAL/register-map and effect-model passages, not external review',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.md']},limitations=['Published ISO and physical evidence remain separate']))
 print('PASS three original layers criterion objects preserved and attached; additive attachment idempotent')
def package():
 from check_layers_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_layers_package.py',ROOT/PROJECT,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[];d=read(ROOT/PROJECT);assert d['criteria']==original()['criteria']
 for c in [v for v in d['criteria'] if v['objective'].startswith('T28.11')]:
  key=c['objective'][4:];v=c['variants'][0]
  labels=['gcc-debug service execute 0','gcc-release service execute 0','clang-release service execute 0','gcc-debug independent oracle',*SELECTORS[key]]
  for delayed in [False,True]:
   artifacts=['labs/peripheral_depth/layers/CONTRACTS.md','assessor/peripheral_depth/layers/ANALYSIS.md','research/v5/expansion/legacy_advanced_layers/objective_mapping.json','labs/peripheral_depth/layers/BOARD_CAPTURE.md']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/peripheral_depth/layers/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Published ISO source access remains separately registered.','Actual ISR/board integration must satisfy T29 and capture prerequisites; no physical UART/device ordering or latency inferred.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T28-06 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T28.12 and T29 remain required authoring/implementation work.','Physical board, published ISO, independent learners/delayed and external review remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS six exact layers author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
