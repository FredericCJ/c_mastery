"""Attach and bind exact original diagnostic criteria; keep external evidence separate."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_diagnostic';PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-07.json';SUPPORT='research/v5/expansion/legacy_advanced_diagnostic/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_diagnostic/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json','cost_analysis.json']]
RATIONALE={'12.a':'Same diagnostic core through actual POSIX UDP, original ED frame/descriptor providers, full-speed packet/request model and selected Linux gadget/usbfs cancellation; interfaces and physical prerequisites kept separate.', '12.b':'Staged big-endian codec with actual FCursor, exhaustive65536-sequence independent wire oracle, strict replay/exhaustion, all splits and coalesced stream recovery boundaries.', '12.c':'Four evidence columns, changed-PHY/routing dependency invalidation and ten wrong-layer/vacuity refusals, complete dedicated-board measurement protocol.'}
SELECTORS={'12.a':['gcc-debug host 0 execute','gcc-debug host 1 execute','gcc-debug core 4 execute','gcc-debug core 5 execute','gcc-debug usb 0 execute','gcc-debug usb 5 execute','Linux gadget 0','Linux gadget 1','Linux gadget 2','Linux gadget stub'], '12.b':['gcc-debug core 1 execute','gcc-debug core 2 execute','gcc-debug core 3 execute','PPC64-O3 core 0 execute','clang-O3 core 3 execute'], '12.c':['changed hardware evidence']}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/peripheral_depth/diagnostic').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public += ['labs/foundations/include/foundation.h','labs/foundations/src/foundation.c','labs/ethernet_depth/include/ethernet_depth.h','labs/ethernet_depth/src/ethernet_depth.c']
 put(HERE/'attachment_PR-T28-07.json',dict(project=d['project'],domain='T28',key='diagnostic_transport_successor',supplied_artifacts=public,shared_contracts=['labs/peripheral_depth/diagnostic/CONTRACTS.md','labs/peripheral_depth/diagnostic/guest/README.md','labs/peripheral_depth/diagnostic/BOARD_CAPTURE.md'],source_register='research/v5/expansion/legacy_advanced_diagnostic/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria'] if c['objective'].startswith('T28.12')],scope='Three T28.12 ordinary and three unchanged delayed tasks; actual FCursor/ED/POSIX/Linux providers; T29/T38/T39 and physical/external work separate'))
 x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/PROJECT,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/peripheral_depth/diagnostic/CONTRACTS.md',learner_artifacts=['diagnostic.c','ethernet.c','usb.c','host.c','guest/dg_gadget.c','criterion-indexed wire/ownership/four-column dossier'],assessment='assessor/peripheral_depth/diagnostic/ANALYSIS.md#'+c['objective'][4:],source_register='research/v5/expansion/legacy_advanced_diagnostic/resources.json') for c in d['criteria'] if c['objective'].startswith('T28.12')],remaining='T29 and T38/T39 physical adapters remain required; physical/source/external dimensions separate'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',scope='Exact USB/POSIX/Linux/frame/descriptor source passages, not external review',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.json']},limitations=['Published ISO and physical evidence remain separate']))
 print('PASS three original diagnostic criterion objects preserved and attached; additive attachment idempotent')
def package():
 from check_diagnostic_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_diagnostic_package.py',ROOT/PROJECT,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[];d=read(ROOT/PROJECT);assert d['criteria']==original()['criteria']
 for c in [v for v in d['criteria'] if v['objective'].startswith('T28.12')]:
  key=c['objective'][4:];v=c['variants'][0]
  labels=['gcc-debug core 0 execute','gcc-release core 0 execute','clang-release core 0 execute','gcc-debug independent oracle',*SELECTORS[key]]
  for delayed in [False,True]:
   artifacts=['labs/peripheral_depth/diagnostic/CONTRACTS.md','assessor/peripheral_depth/diagnostic/ANALYSIS.md','research/v5/expansion/legacy_advanced_diagnostic/objective_mapping.json','labs/peripheral_depth/diagnostic/BOARD_CAPTURE.md']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/peripheral_depth/diagnostic/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Published ISO source access remains separately registered.','Actual ISR/board integration must satisfy T29/T39 and capture prerequisites; no physical Ethernet/USB signaling or latency inferred.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T28-07 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T29 and T38/T39 integration remain required; exact USB source/board routing gaps retained.','Physical board, published ISO, independent learners/delayed and external review remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS six exact diagnostic author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
