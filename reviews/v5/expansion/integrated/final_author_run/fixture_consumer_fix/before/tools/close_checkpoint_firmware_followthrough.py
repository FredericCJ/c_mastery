"""Bind exact retained P4 tasks to executed successor mechanisms, preserving history."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_checkpoint_firmware.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_checkpoint_firmware';PROJECTS=['PR-P4-01','PR-P4-02'];SUPPORT='research/v5/expansion/legacy_checkpoint_firmware/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_checkpoint_firmware/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'T27.08.b': ['emulated-reset', 'board:gcc-O3-mode1-trace1-ring4-off0-ccm', 'bad-vector'], 'T28.05.c': ['gcc-debug:ordinary', 'clang-asan:half-11', 'clang-asan:half-12', 'gcc:register-1', 'board:clang-O3-mode2-trace1-ring4-off4000'], 'T29.06.b': ['gcc-debug:ordinary', 'clang-asan:variants', 'gcc-debug:core-33'], 'T29.06.c': ['timing', 'capture', 'decoder', 'board:clang-O3-mode0-trace1-ring2-off4000'], 'T29.09.b': ['gcc-debug:variants', 'clang-asan:core-37', 'capture'], 'T30.05.b': ['timing', 'retained-timing'], 'T30.05.c': ['capture', 'decoder', 'board:clang-O0-mode1-trace0-ring4-off0', 'board:clang-O3-mode1-trace1-ring4-off0'], 'T31.10.c': ['gcc-debug:variants', 'clang-asan:core-36', 'capture']}
RATIONALE={'T27.08.b': 'Actual poisoned reset/relocation guests and six-object successor F439 map/vector/helper/stack audits with bad CCM/vector rejection.', 'T28.05.c': 'Polling/DMA ownership, stale/stalled inputs, actual halfduplex component/mutants and cross-built qualified-cycle direction adapter; reserved status-bit correction discriminated.', 'T29.06.b': 'Literal N4 FIFO/drop trace, independent13122 histories at N4/N2, two-producer lost-reservation countermodel and separate-ring correction.', 'T29.06.c': 'Complete capture adapter/ABI/decoder/instrument protocol and independent admission/latency predictions. Physical observations remain not-run.', 'T29.09.b': 'Four atomic split-counter rollover cuts, half-range rejection, bounded retry/refusal and sequence retirement; capture correlation/wire timestamp rejections.', 'T30.05.b': 'Added higher-priority workload with independent unit-step schedule,74to89us model; finite-capacity loss/service comparison and overload control.', 'T30.05.c': 'Source-bound raw capture consistency/CRC/clock/stage/warmup/loss checker and trace/optimization image variants. No invented physical samples.', 'T31.10.c': '100 failed acknowledgements, first-fault preservation, late/stale completion isolation, retry retirement and retained-resource stop; board fault record implemented.'}

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def card(project):return 'curriculum/v5/expansion/legacy_assessments/'+project+'.json'
def original(project):return json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+card(project)],cwd=ROOT,text=True,encoding='utf8'))
def historical_equal(now,old):
 def criteria(d):return [{k:v for k,v in c.items() if k!='successor_support'} for c in d['criteria']]
 return criteria(now)==criteria(old) and now['baseline_binding']==old['baseline_binding']
def integrate():
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/checkpoint_firmware_followthrough').iterdir()) if p.is_file()]
 public+=['labs/checkpoint_depth/include/checkpoint_firmware.h','labs/checkpoint_depth/include/checkpoint_depth.h','labs/checkpoint_depth/src/checkpoint_firmware.c','labs/checkpoint_depth/src/checkpoint_depth.c','labs/checkpoint_depth/tests/firmware.c','labs/checkpoint_depth/transfer/halfduplex.h']
 mapping=[]
 for project in PROJECTS:
  d=read(ROOT/card(project));old=original(project);assert historical_equal(d,old)
  put(HERE/('attachment_'+project+'.json'),dict(project=project,domain=d['domain'],key='firmware_followthrough_successor',supplied_artifacts=public,shared_contracts=['labs/checkpoint_firmware_followthrough/CONTRACTS.md'],source_register='research/v5/expansion/legacy_checkpoint_firmware/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Exact original P4 ordinary/delayed tasks with capture/timing/changed-condition and reserved-status correction; actual learner/physical evidence separate'))
  x=attach(d);assert x['criteria']==d['criteria'] and historical_equal(x,old) and attach(copy.deepcopy(x))==x;put(ROOT/card(project),x)
  for c in d['criteria']:mapping.append(dict(project=project,criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/checkpoint_firmware_followthrough/CONTRACTS.md',learner_artifacts=['own full P3/firmware/halfduplex providers','reset/ownership/clock proof and fault traces','independent timing/loss model','source-bound image and capture dossier'],assessment='assessor/checkpoint_firmware_followthrough/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective']],source_register='research/v5/expansion/legacy_checkpoint_firmware/resources.json'))
 put(HERE/'objective_mapping.json',dict(criteria=mapping,remaining='P2/P3/P5/P6/CAP and other recursive inventory obligations remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md',HERE/'source_access.json']},limitations=['N1570 C11 draft is not published C17.','Cross-build, emulator execution, synthetic captures and physical observation are separate.']))
 print('PASS16 retained tasks; additive idempotent attachment')
def package():
 from check_checkpoint_firmware_followthrough import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_checkpoint_firmware_package.py',ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py'}
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
    artifacts=['labs/checkpoint_firmware_followthrough/CONTRACTS.md','assessor/checkpoint_firmware_followthrough/ANALYSIS.md','assessor/checkpoint_firmware_followthrough/DELAYED.md','research/v5/expansion/legacy_checkpoint_firmware/objective_mapping.json']
    dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=SELECTORS[c['objective']],analysis='assessor/checkpoint_firmware_followthrough/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[c['objective']],remaining_author_work=[],external_blockers=[dict(dimension='physical_observation',missing='Qualified dedicated F439 board/package/schematic, calibrated instrument and authorized operator; PA0 regulator-mode restriction must be checked',procedure='labs/checkpoint_firmware_followthrough/BOARD_CAPTURE.md'),dict(dimension='external_review',missing='Non-author technical reviewer'),dict(dimension='independent_learner',missing='Independent learner attempt and retained minimum seven-day delayed attempt')],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='P4 eight ordinary and eight unchanged delayed tasks; implemented capture/timing/variants, physical observation separate',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['P2/P3/P5/P6/CAP and other recursive inventory authoring remain required.','External review, independent/delayed learner and physical observations remain separate.'],summary=dict(criteria=8,delayed=8,commands=len(report['commands']),assertions=len(report['checks']))));print('PASS16 exact P4 dispositions')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
