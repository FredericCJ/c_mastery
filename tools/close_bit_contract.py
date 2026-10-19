"""Attach and evidence-bind nine retained bit-contract tasks and transfers."""
from pathlib import Path
import argparse,copy,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_bit_contract.author_support import attach
HERE=ROOT/'research/v5/expansion/legacy_bit_contract';PROJECT='PR-T04-02';CARD='curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json'
SUPPORT='research/v5/expansion/legacy_bit_contract/completion_dispositions.json';REVIEW='reviews/v5/expansion/legacy_bit_contract/'
REPORTS=[REVIEW+n for n in ['current_validation.json','support_validation.json','exports.json']]
SELECTORS={'05.a': ['gcc-debug:0', 'gcc-debug:1', 'clang:target-O3'], '05.b': ['gcc-debug:2', 'gcc-debug:oracle', 'clang-asan:oracle'], '05.c': ['gcc-debug:3', 'gcc-debug:oracle', 'gcc:intrinsic:False', 'gcc:intrinsic:True', 'clang:intrinsic:False', 'clang:intrinsic:True'], '06.a': ['gcc-debug:0', 'arm-none-eabi-gcc:target-O0'], '06.b': ['gcc-debug:4', 'clang-asan:0', 'clang:target-O0'], '06.c': ['gcc:layout-O0:0', 'clang:layout-O3:1', 'gcc-debug:0', 'gcc:analysis-only'], '07.a': ['gcc:layout-O0:0', 'gcc:layout-O0:1', 'clang:layout-O3:0', 'clang:layout-O3:1'], '07.b': ['gcc-debug:5', 'clang-asan:0', 'clang:target-O3'], '07.c': ['gcc:plugin:1:clang', 'gcc:plugin:2:clang', 'clang:plugin:3:gcc', 'clang:plugin:4:gcc', 'gcc-debug:6', 'gcc:layout-O0:0']}
RATIONALE={'05.a': 'Actual logical16 count boundaries/zero rotation and logical8 masking inside wider carrier, plus wrong unmasked-input mutant.', '05.b': 'Actual half-open fields, full/empty/excess-value boundaries, staged sequential edits and conflicting/agreed simultaneous overlaps; independent bit-list oracle.', '05.c': 'Actual zero/pop/width/leading policy, all8-bit cases and capability-checked intrinsic1048576 comparisons with defined zero-policy mutant.', '06.a': 'Actual BIG/LITTLE codec vectors/inverses through the actual completed integer cell provider.', '06.b': 'Actual offsets0/1/3 and every short length, two separately owned spans and unchanged publication; no cross-object pointers.', '06.c': 'Actual native packed/reordered objects versus explicit wire decoding, unsafe overlay compile-only, header-only unknown skip/reject before payload.', '07.a': 'Eight actual GCC/Clang O0/O3 packing observations of3/5/9 fields and changed underlying unsigned-long type; no portable padding/layout goldens.', '07.b': 'Actual original W1C/RTC state transitions, unrequested-clear RMW defect, no-read repair and mutant; no physical register claim.', '07.c': 'Actual ABI/wire manifests, eight separately compiled shared-library providers and sixteen typed consumers with exact available allocations; old/new/unknown/truncated size/version negotiation without unavailable-tail reads.'}


def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def original():return json.loads(subprocess.check_output(['git','show','HEAD:'+CARD],cwd=ROOT,text=True,encoding='utf8'))
def integrate():
 d=read(ROOT/CARD);old=original();assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding']
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/bit_contract').iterdir()) if p.is_file()]+['labs/integer_kernel/integer.h']
 put(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T04',key='bit_contract_successor',supplied_artifacts=public,shared_contracts=['labs/bit_contract/CONTRACTS.md'],source_register='research/v5/expansion/legacy_bit_contract/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Nine unchanged ordinary and nine delayed tasks; bit/codec/register/ABI mechanisms, independent bit-list oracle and actual prior-provider integration; no learner/physical claim'))
 x=attach(d);assert x['criteria']==old['criteria'] and attach(copy.deepcopy(x))==x;put(ROOT/CARD,x)
 put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching='labs/bit_contract/CONTRACTS.md',learner_artifacts=['learner.c','criterion-indexed bit/wire/register/ABI dossier','criterion-indexed diagnosis and transfer dossier'],assessment='assessor/bit_contract/ANALYSIS.md#'+c['objective'].lower().replace('.',''),command_groups=SELECTORS[c['objective'][4:]],source_register='research/v5/expansion/legacy_bit_contract/resources.json') for c in d['criteria']],remaining='Other T02–06 and recursive-inventory tasks remain required.'))
 put(HERE/'research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in read(HERE/'resources.json')['resources']],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [HERE/'resources.json',HERE/'audit.md']},limitations=['N1570 C11 draft is not published C17.','C17 core plus actual compiler/ABI comparisons; synthetic register behavior, not physical MMIO.']))
 print('PASS nine original criteria retained; additive attachment idempotent')
def package():
 from check_bit_contract import validate
 report=read(ROOT/REPORTS[0]);assert not validate(report)
 files={Path(__file__),ROOT/'tools/check_bit_contract_package.py',ROOT/CARD}
 for p in REPORTS:
  r=read(ROOT/p);assert r['passed'];files.add(ROOT/p)
  for f,h in r.get('input_sha256',{}).items():assert sha(ROOT/f)==h,(p,f);files.add(ROOT/f)
 files.update(p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json')
 for r in read(HERE/'resources.json')['resources']:
  for f,h in r['inspection']['source_sha256'].items():assert sha(ROOT/f)==h,('source changed',f);files.add(ROOT/f)
 d=read(ROOT/CARD);assert d['criteria']==original()['criteria'];rows=[]
 for c in d['criteria']:
  key=c['objective'][4:];v=c['variants'][0];groups=SELECTORS[key]
  for delayed in [False,True]:
   artifacts=['labs/bit_contract/CONTRACTS.md','assessor/bit_contract/ANALYSIS.md','research/v5/expansion/legacy_bit_contract/objective_mapping.json']
   dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts[:1]),('implementation','complete',artifacts),('author_validation','passed',REPORTS),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
   rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=artifacts,author_validation=dict(report=REPORTS[0],command_groups=groups,analysis='assessor/bit_contract/ANALYSIS.md#'+c['objective'].lower().replace('.','')),closure_rationale=RATIONALE[key],remaining_author_work=[],evidence_limits=report['limits']))
 put(ROOT/SUPPORT,dict(schema_version=1,scope='PR-T04-02 nine ordinary and nine unchanged delayed tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=REPORTS,updates=rows,remaining_parents=['Other foundations and recursive inventory authoring remains required.','External review, independent/delayed learner and physical evidence remain separate.'],summary=dict(criteria=9,delayed=9,commands=len(report['commands']),assertions=len(report['checks']),scope='Separate logical bits, span/codec protocols, original register state machine, actual ABI/plugin observations and strict target closure')));print('PASS18 exact bit-contract dispositions; external evidence remains separate')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
