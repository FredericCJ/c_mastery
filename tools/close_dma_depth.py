"""Attach and bind exact original dma criteria; keep external evidence separate."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_advanced.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_advanced_dma';PROJECT='curriculum/v5/expansion/legacy_assessments/PR-T28-06.json';SUPPORT='research/v5/expansion/legacy_advanced_dma/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_advanced_dma/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json','cost_analysis.json']]
RATIONALE={'10.a':'Bounded bank/alignment/capacity/overlap admission and copied software scatter/gather before publication, selected DMA2 ports and strict closure.', '10.b':'FIFO quiescence, conservative error progress, once-only terminal generation and independently derived idempotent memory-copy suffixes.', '10.c':'Actual B06 cache/peer providers, cached H745 SRAM3 and two-COREID HSEM1 polling/acknowledgement lease with original cache and register models.'}
SELECTORS={'10.a':['gcc-debug execute 1','gcc-debug execute 2','gcc-debug target0 execute 1','gcc-debug target0 execute 2','gcc-debug target0 execute 4','f439-arm-none-eabi-gcc-O3 combined audit link'], '10.b':['gcc-debug execute 4','gcc-debug execute 5','gcc-debug target0 execute 3','gcc-debug composition execute'], '10.c':['gcc-debug cache execute 3','gcc-debug cache execute 6','gcc-debug notice execute 0','gcc-debug notice execute 1','gcc-debug notice execute 2','gcc-debug notice execute 3','gcc-debug independent notice-oracle','gcc-debug port0 execute 0','gcc-debug port1 execute 0','gcc-debug port1 execute 1','gcc-debug port1 execute 2','gcc-debug port1 execute 3','h745-m7-clang-O3 combined audit link','h745-m4-clang-O3 notice_port disassembly','capture analysis']}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+PROJECT],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/PROJECT);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/peripheral_depth/dma').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
 public+=['labs/firmware_io/include/firmware_io.h','labs/firmware_io/src/firmware_io.c','labs/firmware_io/CONTRACTS.md']+['labs/firmware_lifecycle/targets/'+n for n in ['h745_cache.h','h745_cache.c','h745_port.c','H745_CACHE.md','H745_ERRATA.md']]
 put(HERE/'attachment_PR-T28-06.json',dict(project=d['project'],domain='T28',key='dma_ownership_successor',supplied_artifacts=public,shared_contracts=['labs/peripheral_depth/dma/CONTRACTS.md','labs/firmware_io/CONTRACTS.md','labs/firmware_lifecycle/targets/H745_CACHE.md'],source_register='research/v5/expansion/legacy_advanced_dma/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria'] if c['objective'].startswith('T28.10')],scope='Three T28.10 ordinary and three unchanged delayed tasks; actual fw_dma/fw_cache_range/fw_peer_step providers; T28.11–.12 and physical/external work separate'))
 x=attach(d);assert attach(copy.deepcopy(x))==x and x['criteria']==old['criteria'];put(ROOT/PROJECT,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/peripheral_depth/dma/CONTRACTS.md',learner_artifacts=['dma.c','target.c','notice.c','notice_port.c','criterion-indexed ownership/visibility/partial-count dossier'],assessment='assessor/peripheral_depth/dma/ANALYSIS.md#'+c['objective'][4:],source_register='research/v5/expansion/legacy_advanced_dma/resources.json') for c in d['criteria'] if c['objective'].startswith('T28.10')],remaining='T28.11–.12 and T29 remain required; physical/source/external dimensions separate'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',scope='Exact T28 DMA/FIFO/cache/HSEM passages, not external review',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'source_access.json',HERE/'audit.md']},limitations=['Published ISO and physical evidence remain separate']))
 print('PASS three original DMA criterion objects preserved and attached; additive attachment idempotent')
def package():
 from check_dma_depth import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_dma_package.py',ROOT/PROJECT,ROOT/'research/v5/expansion/legacy_advanced/author_support.py'}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json');rows=[];d=read(ROOT/PROJECT);assert d['criteria']==original()['criteria']
 for c in [v for v in d['criteria'] if v['objective'].startswith('T28.10')]:
  key=c['objective'][4:];v=c['variants'][0]
  labels=['gcc-debug execute 0','gcc-release execute 0','clang-release execute 0','gcc-debug independent oracle',*SELECTORS[key]]
  for delayed in [False,True]:
   artifacts=['labs/peripheral_depth/dma/CONTRACTS.md','assessor/peripheral_depth/dma/ANALYSIS.md','research/v5/expansion/legacy_advanced_dma/objective_mapping.json','labs/peripheral_depth/dma/BOARD_CAPTURE.md']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=d['project'],objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],checks=labels,analysis='assessor/peripheral_depth/dma/ANALYSIS.md#'+key),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']+['Published ISO source access remains separately registered.','Actual ISR/board integration must satisfy T29 and capture prerequisites; no physical DMA/cache/HSEM observation or peer latency inferred.']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T28-06 three ordinary and three unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['T28.11–.12 and T29 remain required authoring/implementation work.','Physical board, published ISO, independent learners/delayed and external review remain separate.'],summary=dict(criteria=3,delayed=3,checks=len(report['checks']))));print('PASS six exact dma author dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
