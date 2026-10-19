"""Preserve all nine original T39.06–.08 criterion/changed task pairs."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_ethernet_ring';PROJECTS=['PR-T39-05','PR-T39-06','PR-T39-07'];REPORT='research/v5/expansion/legacy_ethernet_ring/validation.json';SUPPORT='research/v5/expansion/legacy_ethernet_ring/completion_dispositions.json'
from research.v5.expansion.legacy_ethernet_ring.author_support import attach
ARTIFACTS=['labs/ethernet_followthrough/ring/CONTRACTS.md','labs/ethernet_followthrough/ring/BOARD_CAPTURE.md','assessor/ethernet_followthrough/ring/ANALYSIS.md','assessor/ethernet_followthrough/ring/ring.c','assessor/ethernet_followthrough/ring/target.c','assessor/ethernet_followthrough/ring/variants.c','labs/ethernet_followthrough/ring/tests.c','labs/ethernet_followthrough/ring/mapped.c','labs/ethernet_followthrough/ring/variants_tests.c','labs/ethernet_followthrough/ring/guest.c','assessor/ethernet_followthrough/ring/oracle.py']
REMAINING=['Physical selected-board/PHY/RMII clock, DMA process behavior and frame/peer captures remain required.','Changed CSTF silicon-status/capture mapping must be established before using that separately declared hardware configuration.','Other T38–40 and mixed LRUN parents remain required, not closed by these nine criterion pairs.','External review and independent original/seven-day learner attempts remain unperformed.']
PATTERNS={
 'T39.06.a':['.init','.bad-placement','.closure','.stack'],
 'T39.06.b':['.init','.order','.bounds','.failures','.port','.mutation.MODE','.mutation.RESET','.mutation.ORDER'],
 'T39.06.c':['.init','.visibility','.port.audit','.mutation.CLEAN'],
 'T39.07.a':['.rx','.chain','.mutation.FCS','.mutation.STRIP'],
 'T39.07.b':['.drop','.chain','.mutation.RETAINED_RX'],
 'T39.07.c':['.rx','.consumer','.mutation.BORROW'],
 'T39.08.a':['.tx','.publication','.mutation.REBUILD','.mutation.CHAIN','.mutation.RETAINED_TX'],
 'T39.08.b':['.tx','.publication','.mutation.COMPLETE'],
 'T39.08.c':['.recovery','.bounds','.failures','.publication','.mutation.STOP','.mutation.PHY','.mutation.RETAINED_ABORT']}
WHY={
 'T39.06.a':'Actual imported BuRange whole-interval admission, live typed section layout and deliberate invalid CCM link rejection before publication.',
 'T39.06.b':'Actual original ordered RCC/RMII/GPIO/MMIO port, separate SWR lifecycle, exact dependency-edge checker and27 callback cut points.',
 'T39.06.c':'Actual reachability/barrier object audit and distinct two-line CPU/device visibility experiment, including dirty-neighbor ownership and lost-write counterexample.',
 'T39.07.a':'Complete single-descriptor FL/FCS adapter and explicitly scoped changed CSTF capture normalization; no changed silicon FL guess.',
 'T39.07.b':'Actual retained two-descriptor validator, incomplete/late-LS/drop recovery and24 capacity splits with final-length authority.',
 'T39.07.c':'Actual held-token ring exhaustion/duplicate/stale release and synthetic upper rejection with exactly-once release and separate layer counters.',
 'T39.08.a':'Actual normal-ring control reconstruction plus retained two-descriptor and new three-descriptor final-head publication models.',
 'T39.08.b':'Actual copy-before-success and separate real-pointer borrowed-chain contract; returned ownership/error versus IRQ/delivery distinctions.',
 'T39.08.c':'Actual two-slot backpressure, link incarnation, unknown-stop retention, reset/read-only retry, nonwrapping identities and partial-publication borrowed recovery.'}
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon(p):return ROOT/('curriculum/v5/expansion/legacy_assessments/'+p+'.json')
def all_rows(r):
 rows=[]
 for p in PROJECTS:
  for c in read(canon(p))['criteria']:
   v=c['variants'][0]
   for delayed in [False,True]:
    dimensions={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:2]),('implementation','complete',ARTIFACTS),('author_validation','passed',[REPORT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
    checks=[q['id'] for q in r['commands'] if any(x in q['id'] for x in PATTERNS[c['objective']]) or q['id'] in [cc+'.O'+o+'.execute' for cc in ['arm-none-eabi-gcc','clang'] for o in ['0','3']]]
    rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=p,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions,artifacts=ARTIFACTS,author_validation=dict(report=REPORT,checks=checks),closure_rationale=WHY[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 return rows
def integrate():
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/ethernet_followthrough/ring').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/ethernet_depth/include/ethernet_depth.h','labs/ethernet_depth/src/ethernet_depth.c','labs/ethernet_depth/CONTRACTS.md','labs/interrupt_depth/frame/target.S','labs/bringup_depth/bringup.h','labs/bringup_depth/bringup.c','labs/bringup_depth/CONTRACTS.md',*['labs/ethernet_followthrough/phy/'+n for n in ['phy.h','learner.c','model.h','model.c','CONTRACTS.md']]]
 for p in PROJECTS:
  d=read(canon(p));old=HERE/(p+'.before-ring.json')
  if not old.exists():old.write_bytes(canon(p).read_bytes())
  write(HERE/('attachment_'+p+'.json'),dict(project=p,domain='T39',supplied_artifacts=public,shared_contracts=ARTIFACTS[:2]+['labs/ethernet_depth/CONTRACTS.md'],source_register='research/v5/expansion/legacy_ethernet_ring/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Original criterion/V1 pairs; actual normal-ring producer plus explicitly separate changed-condition models; physical and independent gates remain.'))
  after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(p),after)
 print('PASS additive ring attachments')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_ethernet_ring.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
 for r in reports:
  assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
  for k in ['source_sha256','outputs_sha256']:
   for n,h in r[k].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in r['source_sha256']}
 for p in PROJECTS:
  d=read(canon(p));old=read(HERE/(p+'.before-ring.json'));assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon(p))
 r,e=reports;rows=all_rows(r);assert len(rows)==18
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='PR-T39-05..07 all nine original criterion/V1 pairs; six evidence dimensions',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_ethernet_ring/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=9,delayed=9,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS18 exact ring dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
